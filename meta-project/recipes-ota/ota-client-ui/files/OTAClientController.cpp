#include "OTAClientController.h"
#include <QDebug>

using FlashState = ::v1::com::ota::OTAFlashService::FlashState;
using OTAErrorCode = ::v1::com::ota::OTAFlashService::OTAErrorCode;

OTAClientController::OTAClientController(QObject *parent) : QObject(parent) {
  m_pollTimer = new QTimer(this);
  m_pollTimer->setInterval(500);
  connect(m_pollTimer, &QTimer::timeout, this, [this]() {
    if (!m_proxy)
      connectProxy();
  });

  connect(
      this, &OTAClientController::availabilityChangedSignal, this,
      [this](int status) {
        onAvailabilityChanged(
            static_cast<CommonAPI::AvailabilityStatus>(status));
      },
      Qt::QueuedConnection);

  connect(
      this, &OTAClientController::flashProgressSignal, this,
      [this](int fs, int pct, int ec) {
        onFlashProgressEvent(
            "", static_cast<FlashState>(static_cast<FlashState::Literal>(fs)),
            static_cast<uint8_t>(pct),
            static_cast<OTAErrorCode>(static_cast<OTAErrorCode::Literal>(ec)));
      },
      Qt::QueuedConnection);
}

void OTAClientController::initialize() {
  m_runtime = CommonAPI::Runtime::get();
  m_pollTimer->start();
}

void OTAClientController::connectProxy() {
  m_proxy = m_runtime->buildProxy<::v1::com::ota::OTAFlashServiceProxy>(
      "local", "com.ota.OTAFlashService", "OTAClientUI");

  if (!m_proxy) {
    qDebug() << "buildProxy failed, retrying...";
    return;
  }

  m_pollTimer->stop();
  qDebug() << "Proxy created, isAvailable:" << m_proxy->isAvailable();

  m_proxy->getProxyStatusEvent().subscribe(
      [this](CommonAPI::AvailabilityStatus status) {
        emit availabilityChangedSignal((int)status);
      });

  m_proxy->getFlashProgressEvent().subscribe(
    [this](const std::string &tid,
           const FlashState &fs,
           const uint8_t &pct,
           const OTAErrorCode &ec) {
        qDebug() << "FlashProgress event received, state:" << (int)fs << "pct:" << (int)pct;
        emit flashProgressSignal((int)fs, (int)pct, (int)ec);
    });

  // check availability immediately in case service is already up
  if (m_proxy->isAvailable()) {
    emit availabilityChangedSignal(
        (int)CommonAPI::AvailabilityStatus::AVAILABLE);
  } else {
    emit availabilityChangedSignal(
        (int)CommonAPI::AvailabilityStatus::NOT_AVAILABLE);
  }
}

void OTAClientController::onAvailabilityChanged(
    CommonAPI::AvailabilityStatus status) {
  bool avail = (status == CommonAPI::AvailabilityStatus::AVAILABLE);
  setServiceAvailable(avail);
  if (avail) {
    setState(Waiting);
    setStatusMessage("Service connected. Ready.");
  } else {
    setState(Waiting);
    setStatusMessage("Waiting for OTA service…");
  }
}

void OTAClientController::onFlashProgressEvent(
    const std::string & /*transferId*/, const FlashState &fs,
    const uint8_t &pct, const OTAErrorCode &ec) {
  setProgress(pct);

  switch (fs) {
  case FlashState::IDLE:
    setState(Waiting);
    setStatusMessage("Idle");
    break;
  case FlashState::RECEIVING:
    setState(Receiving);
    setStatusMessage("Receiving firmware…");
    setErrorMessage({});
    break;
  case FlashState::FLASHING:
    setState(Receiving);
    setStatusMessage("Writing to flash…");
    break;
  case FlashState::COMPLETE:
    setState(Complete);
    setStatusMessage("Update complete. Rebooting…");
    break;
  case FlashState::FAILED:
    setState(Failed);
    setStatusMessage("Transfer failed");
    break;
  }

  if (ec != OTAErrorCode::SUCCESS) {
    static const QMap<int, QString> errMap = {
        {(int)OTAErrorCode::CHECKSUM_MISMATCH, "Checksum mismatch"},
        {(int)OTAErrorCode::FLASH_WRITE_ERROR, "Flash write error"},
        {(int)OTAErrorCode::TRANSFER_TIMEOUT, "Transfer timed out"},
        {(int)OTAErrorCode::INCOMPLETE_TRANSFER, "Incomplete transfer"},
        {(int)OTAErrorCode::INVALID_TRANSFER_ID, "Invalid transfer ID"},
    };
    setErrorMessage(errMap.value((int)ec, "Unknown error"));
  }
}

void OTAClientController::setState(int s) {
  if (m_state == s)
    return;
  m_state = s;
  emit stateChanged();
}
void OTAClientController::setProgress(int p) {
  if (m_progress == p)
    return;
  m_progress = p;
  emit progressChanged();
}
void OTAClientController::setStatusMessage(const QString &msg) {
  if (m_statusMessage == msg)
    return;
  m_statusMessage = msg;
  emit statusMessageChanged();
}
void OTAClientController::setErrorMessage(const QString &msg) {
  if (m_errorMessage == msg)
    return;
  m_errorMessage = msg;
  emit errorMessageChanged();
}
void OTAClientController::setServiceAvailable(bool v) {
  if (m_serviceAvailable == v)
    return;
  m_serviceAvailable = v;
  emit serviceAvailableChanged();
}