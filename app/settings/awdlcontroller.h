#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <Security/Authorization.h>

class AwdlController : public QObject
{
    Q_OBJECT

public:
    explicit AwdlController(QObject *parent = nullptr);
    ~AwdlController();

    bool requestAdminAuthorization();
    bool hasValidAuthorization() const;
    bool startAwdlControl();
    bool stopAwdlControl();
    bool isAwdlEnabled() const;

signals:
    void authorizationChanged(bool hasAuth);
    void errorOccurred(const QString &error);

private slots:
    void onRouteSocketReady();

private:
    bool setAwdlInterfaceState(bool enable);
    bool executePrivilegedIfconfigCommand(const QString &command);
    bool checkAwdlState() const;
    void clearAuthorization();
    bool setupRouteSocket();
    void closeRouteSocket();
    
    AuthorizationRef m_authRef;
    bool m_hasValidAuth;
    bool m_controlActive;
    int m_routeSocket;
    QSocketNotifier* m_socketNotifier;
};