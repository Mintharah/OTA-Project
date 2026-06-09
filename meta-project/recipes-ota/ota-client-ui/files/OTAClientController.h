#ifndef OTACLIENTCONTROLLER_H
#define OTACLIENTCONTROLLER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>
#include <CommonAPI/CommonAPI.hpp>
#include <v1/com/ota/OTAFlashServiceProxy.hpp>

class OTAClientController : public QObject {
    Q_OBJECT

    Q_PROPERTY(int     state            READ state            NOTIFY stateChanged)
    Q_PROPERTY(int     progress         READ progress         NOTIFY progressChanged)
    Q_PROPERTY(QString statusMessage    READ statusMessage    NOTIFY statusMessageChanged)
    Q_PROPERTY(QString errorMessage     READ errorMessage     NOTIFY errorMessageChanged)
    Q_PROPERTY(bool    serviceAvailable READ serviceAvailable NOTIFY serviceAvailableChanged)

public:
    enum State { Waiting = 0, Receiving = 1, Complete = 2, Failed = 3 };
    Q_ENUM(State)

    explicit OTAClientController(QObject *parent = nullptr);

    int     state()            const { return m_state; }
    int     progress()         const { return m_progress; }
    QString statusMessage()    const { return m_statusMessage; }
    QString errorMessage()     const { return m_errorMessage; }
    bool    serviceAvailable() const { return m_serviceAvailable; }

public slots:
    void initialize();

signals:
    void stateChanged();
    void progressChanged();
    void statusMessageChanged();
    void errorMessageChanged();
    void serviceAvailableChanged();
    // internal signals for thread safety
    void availabilityChangedSignal(int status);
    void flashProgressSignal(int flashState, int progress, int errorCode);

private:
    void connectProxy();
    void onAvailabilityChanged(CommonAPI::AvailabilityStatus status);
    void onFlashProgressEvent(
        const std::string &transferId,
        const ::v1::com::ota::OTAFlashService::FlashState &flashState,
        const uint8_t &progress,
        const ::v1::com::ota::OTAFlashService::OTAErrorCode &errorCode);

    void setState(int s);
    void setProgress(int p);
    void setStatusMessage(const QString &msg);
    void setErrorMessage(const QString &msg);
    void setServiceAvailable(bool v);

    using Proxy = ::v1::com::ota::OTAFlashServiceProxy<>;
    std::shared_ptr<Proxy>                      m_proxy;
    std::shared_ptr<CommonAPI::Runtime>         m_runtime;

    int     m_state            = Waiting;
    int     m_progress         = 0;
    QString m_statusMessage    = "Waiting for OTA service…";
    QString m_errorMessage;
    bool    m_serviceAvailable = false;

    QTimer *m_pollTimer = nullptr;
};
#endif // OTACLIENTCONTROLLER_H
