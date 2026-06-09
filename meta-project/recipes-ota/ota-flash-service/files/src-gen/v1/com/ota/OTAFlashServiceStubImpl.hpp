#ifndef __OTA_FLASH_SERVICE_STUB_IMPL_HPP__
#define __OTA_FLASH_SERVICE_STUB_IMPL_HPP__

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <sys/statvfs.h>
#include <thread>
#include <unordered_map>
#include <v1/com/ota/OTAFlashServiceStubDefault.hpp>
#include <vector>

#include "FlashManager.hpp"

class OTAFlashServiceStubImpl
    : public v1::com::ota::OTAFlashServiceStubDefault {
public:
  FlashManager flashManager_;

  const CommonAPI::Version &
  getInterfaceVersion(std::shared_ptr<CommonAPI::ClientId> _client) override;

  const ::v1::com::ota::OTAFlashService::FlashState &
  getCurrentStateAttribute(const std::shared_ptr<CommonAPI::ClientId>) override;

  const uint8_t &getFlashProgressPercentAttribute(
      const std::shared_ptr<CommonAPI::ClientId>) override;

  void startTransfer(const std::shared_ptr<CommonAPI::ClientId> _client,
                     uint64_t _imageSize, uint32_t _totalChunks,
                     std::string _expectedChecksum,
                     startTransferReply_t _reply) override;
  void sendChunk(const std::shared_ptr<CommonAPI::ClientId> _client,
                 std::string _transferId, uint32_t _chunkIndex,
                 CommonAPI::ByteBuffer _chunkData,
                 sendChunkReply_t _reply) override;
  void finalizeTransfer(const std::shared_ptr<CommonAPI::ClientId> _client,
                        std::string _transferId,
                        finalizeTransferReply_t _reply) override;
  void cancelTransfer(const std::shared_ptr<CommonAPI::ClientId> _client,
                      std::string _transferId,
                      cancelTransferReply_t _reply) override;

private:
  struct TransferSession {
    uint64_t imageSize;
    uint32_t totalChunks;
    std::string expectedChecksum;
    std::string stagingPath;
    std::vector<CommonAPI::ByteBuffer> chunks;
    std::vector<bool> receivedFlags;
    std::chrono::steady_clock::time_point lastActivity;
    uint32_t chunkSize = 0;
  };

  std::unordered_map<std::string, TransferSession> sessions_;
  std::mutex sessionsMutex_;

  ::v1::com::ota::OTAFlashService::FlashState currentState_{
      ::v1::com::ota::OTAFlashService::FlashState::IDLE};
  uint8_t flashProgressPercent_{0};

  std::string generateTransferId();
  std::string computeSHA256(const std::string &path);
  void zeroPartitionHeader(const std::string &partition);
  void
  handleFlashFailure(const std::string &transferId,
                     ::v1::com::ota::OTAFlashService::OTAErrorCode errorCode);

  std::thread flashThread_;
  std::thread watchdogThread_;
  std::atomic<bool> watchdogRunning_{false};

  void watchdogWorker();
  void flashWorker(std::string transferId);
  std::string getInactivePartition();
};

#endif