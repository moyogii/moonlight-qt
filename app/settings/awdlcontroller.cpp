#include "awdlcontroller.h"
#include <QDebug>
#include <QProcess>
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <vector>

AwdlController::AwdlController(QObject *parent)
    : QObject(parent)
    , m_authRef(nullptr)
    , m_hasValidAuth(false)
    , m_controlActive(false)
    , m_routeSocket(-1)
    , m_socketNotifier(nullptr)
{
}

AwdlController::~AwdlController()
{
    stopAwdlControl();
    closeRouteSocket();
    clearAuthorization();
}

bool AwdlController::requestAdminAuthorization()
{
    clearAuthorization();
    
    OSStatus status = AuthorizationCreate(nullptr, kAuthorizationEmptyEnvironment,
                                         kAuthorizationFlagDefaults, &m_authRef);
    if (status != errAuthorizationSuccess) {
        emit errorOccurred(tr("Failed to create authorization"));
        return false;
    }
    
    AuthorizationItem authItem = { kAuthorizationRightExecute, 0, nullptr, 0 };
    AuthorizationRights authRights = { 1, &authItem };
    
    AuthorizationFlags flags = kAuthorizationFlagInteractionAllowed |
                              kAuthorizationFlagPreAuthorize |
                              kAuthorizationFlagExtendRights;
    
    status = AuthorizationCopyRights(m_authRef, &authRights, nullptr, flags, nullptr);
    
    if (status == errAuthorizationSuccess) {
        m_hasValidAuth = true;
        emit authorizationChanged(true);
        return true;
    }
    
    if (status == errAuthorizationCanceled) {
        emit errorOccurred(tr("Authorization canceled"));
    } else {
        emit errorOccurred(tr("Failed to obtain admin privileges"));
    }
    
    return false;
}

bool AwdlController::hasValidAuthorization() const
{
    return m_hasValidAuth;
}

bool AwdlController::startAwdlControl()
{
    if (m_controlActive) {
        return true;
    }
    
    if (!hasValidAuthorization()) {
        if (!requestAdminAuthorization()) {
            emit errorOccurred(tr("No authorization for AWDL control"));
            return false;
        }
    }
    
    if (!setupRouteSocket()) {
        emit errorOccurred(tr("Failed to setup network monitoring"));
        return false;
    }
    
    m_controlActive = true;
    setAwdlInterfaceState(false); // Disable immediately
    return true;
}

bool AwdlController::stopAwdlControl()
{
    if (!m_controlActive) {
        return true;
    }
    
    closeRouteSocket();
    setAwdlInterfaceState(true);
    m_controlActive = false;
    return true;
}

bool AwdlController::isAwdlEnabled() const
{
    return checkAwdlState();
}


void AwdlController::clearAuthorization()
{
    if (m_authRef) {
        AuthorizationFree(m_authRef, kAuthorizationFlagDefaults);
        m_authRef = nullptr;
    }
    
    if (m_hasValidAuth) {
        m_hasValidAuth = false;
        emit authorizationChanged(false);
    }
}

void AwdlController::onRouteSocketReady()
{
    if (m_routeSocket < 0) {
        return;
    }
    
    char buffer[2048];
    ssize_t n = recv(m_routeSocket, buffer, sizeof(buffer), 0);
    if (n > 0) {
        // Check if AWDL interface came up and disable it immediately
        if (checkAwdlState()) {
            setAwdlInterfaceState(false);
        }
    }
}

bool AwdlController::setAwdlInterfaceState(bool enable)
{
    if (!hasValidAuthorization()) {
        return false;
    }
    
    QString command = enable ? "up" : "down";
    return executePrivilegedIfconfigCommand(QString("awdl0 %1").arg(command));
}

bool AwdlController::executePrivilegedIfconfigCommand(const QString &command)
{
    if (!hasValidAuthorization()) {
        qDebug() << "No authorization for privileged ifconfig command";
        return false;
    }
    
    QStringList parts = command.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return false;
    }
    
    const char* tool = "/sbin/ifconfig";
    
    std::vector<std::string> argStorage;
    for (const QString& part : parts) {
        argStorage.push_back(part.toStdString());
    }
    
    std::vector<char*> argv;
    for (auto& arg : argStorage) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    
    // @TODO: Replace with privledged helper in the future, AuthorizationExecuteWithPrivileges is deprecated but it still works.
    OSStatus status = AuthorizationExecuteWithPrivileges(
        m_authRef, tool, kAuthorizationFlagDefaults, argv.data(), nullptr);
    if (status == errAuthorizationSuccess) {
        qDebug() << "Successfully executed ifconfig command:" << command;
        return true;
    } else {
        qDebug() << "Failed to execute ifconfig command:" << command << "with status:" << status;
        return false;
    }
}

bool AwdlController::setupRouteSocket()
{
    m_routeSocket = socket(AF_ROUTE, SOCK_RAW, 0);
    if (m_routeSocket < 0) {
        qDebug() << "Failed to create route socket";
        return false;
    }
    
    m_socketNotifier = new QSocketNotifier(m_routeSocket, QSocketNotifier::Read, this);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &AwdlController::onRouteSocketReady);
    m_socketNotifier->setEnabled(true);
    
    return true;
}

void AwdlController::closeRouteSocket()
{
    if (m_socketNotifier) {
        m_socketNotifier->setEnabled(false);
        m_socketNotifier->deleteLater();
        m_socketNotifier = nullptr;
    }
    
    if (m_routeSocket >= 0) {
        close(m_routeSocket);
        m_routeSocket = -1;
    }
}


bool AwdlController::checkAwdlState() const
{
    struct ifaddrs *ifaddr, *ifa;
    bool isUp = false;
    
    if (getifaddrs(&ifaddr) == -1) {
        return false;
    }
    
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }
        
        if (strcmp(ifa->ifa_name, "awdl0") == 0) {
            isUp = (ifa->ifa_flags & IFF_UP) != 0;
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    return isUp;
}