#ifndef FLASH_MANAGER_HPP
#define FLASH_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class FlashManager {
public:
  enum class FlashResult {
    SUCCESS,
    ERR_OPEN_DEVICE,
    ERR_WRITE,
    ERR_CHECKSUM_MISMATCH,
    ERR_UBOOT_ENV
  };

  using ProgressCallback = std::function<void(uint8_t percent)>;

  FlashManager();

  // called once chunk received from qnx
  bool writeChunk(uint32_t chunkIndex, const std::vector<uint8_t> &data,
                  uint32_t offset);

  // called after chunks are received, verifies
  FlashResult finalizeAndArm(const std::string &expectedChecksum);

  void setProgressCallback(ProgressCallback cb);
  void setTotalImageSize(uint64_t size) { m_totalImageSize = size; }
private:
  int m_fd{-1};
  uint64_t m_bytesWritten{0};
  ProgressCallback m_progressCb;
  uint64_t m_totalImageSize{0};

  bool openPartition();
  void closePartition();
  std::string computeSHA256();
  bool setBootPartition();
  std::string getInactivePartition();
};

#endif