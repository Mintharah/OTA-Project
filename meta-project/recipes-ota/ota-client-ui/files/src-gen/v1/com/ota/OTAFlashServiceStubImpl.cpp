#include "OTAFlashServiceStubImpl.hpp"
#include <algorithm>
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <openssl/evp.h>
#include <random>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

// Interface Version
const CommonAPI::Version &OTAFlashServiceStubImpl::getInterfaceVersion(
    std::shared_ptr<CommonAPI::ClientId>) {
  static const CommonAPI::Version version(1, 0);
  return version;
}

// Observable attribute getters
const ::v1::com::ota::OTAFlashService::FlashState &
OTAFlashServiceStubImpl::getCurrentStateAttribute(
    const std::shared_ptr<CommonAPI::ClientId>) {
  return currentState_;
}

const uint8_t &OTAFlashServiceStubImpl::getFlashProgressPercentAttribute(
    const std::shared_ptr<CommonAPI::ClientId>) {
  return flashProgressPercent_;
}

// Helper to generate transfer ID
std::string OTAFlashServiceStubImpl::generateTransferId() {
  using namespace std::chrono;
  auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                .count();

  std::random_device rd;
  std::mt19937_64 eng(rd());
  std::uniform_int_distribution<uint64_t> dist;

  std::ostringstream oss;
  oss << "txfr-" << ms << "-" << std::hex << std::setw(16) << std::setfill('0')
      << dist(eng);

  return oss.str();
}

// startTransfer
void OTAFlashServiceStubImpl::startTransfer(
    const std::shared_ptr<CommonAPI::ClientId> /*_client*/, uint64_t imageSize,
    uint32_t totalChunks, std::string expectedChecksum,
    startTransferReply_t reply) {
  using OTAErrorCode = ::v1::com::ota::OTAFlashService::OTAErrorCode;
  using FlashState = ::v1::com::ota::OTAFlashService::FlashState;

  if (imageSize == 0 || totalChunks == 0 || expectedChecksum.empty()) {
    reply("", OTAErrorCode::INVALID_PARAMETER);
    return;
  }

  constexpr uint64_t MAX_IMAGE_BYTES = 4ULL * 1024 * 1024 * 1024;
  constexpr uint32_t MAX_CHUNKS = 65536;

  if (imageSize > MAX_IMAGE_BYTES || totalChunks > MAX_CHUNKS) {
    reply("", OTAErrorCode::INVALID_PARAMETER);
    return;
  }

  std::lock_guard<std::mutex> lock(sessionsMutex_);

  if (currentState_ == FlashState::FLASHING) {
    reply("", OTAErrorCode::TRANSFER_IN_PROGRESS);
    return;
  }

  std::string transferId = generateTransferId();
  TransferSession session;
  session.imageSize = imageSize;
  session.totalChunks = totalChunks;
  session.expectedChecksum = std::move(expectedChecksum);
  session.receivedFlags.resize(totalChunks, false);

  session.chunkSize = (totalChunks > 0) ? (imageSize / totalChunks) : 0;
  flashManager_ = FlashManager(); // reset state
  flashManager_.setTotalImageSize(imageSize);

  sessions_.emplace(transferId, std::move(session));
  sessions_[transferId].lastActivity = std::chrono::steady_clock::now();

  if (!watchdogRunning_.load()) {
    watchdogRunning_ = true;
    watchdogThread_ =
        std::thread(&OTAFlashServiceStubImpl::watchdogWorker, this);
  }

  currentState_ = FlashState::RECEIVING;
  flashProgressPercent_ = 0;

  reply(transferId, OTAErrorCode::SUCCESS);
}

// sendChunk
void OTAFlashServiceStubImpl::sendChunk(
    const std::shared_ptr<CommonAPI::ClientId> /*_client*/,
    std::string transferId, uint32_t chunkIndex,
    CommonAPI::ByteBuffer chunkData, sendChunkReply_t reply) {
  using OTAErrorCode = ::v1::com::ota::OTAFlashService::OTAErrorCode;

  std::lock_guard<std::mutex> lock(sessionsMutex_);

  auto it = sessions_.find(transferId);
  if (it == sessions_.end()) {
    reply(OTAErrorCode::INVALID_TRANSFER_ID);
    return;
  }

  TransferSession &session = it->second;

  if (chunkIndex >= session.totalChunks) {
    reply(OTAErrorCode::CHUNK_OUT_OF_ORDER);
    return;
  }

  if (session.receivedFlags[chunkIndex]) {
    // duplicate, silent success, dont overwrite
    reply(OTAErrorCode::SUCCESS);
    return;
  }

  // calculate offset for this chunk
  uint64_t offset = static_cast<uint64_t>(chunkIndex) * session.chunkSize;
  if (!flashManager_.writeChunk(chunkIndex, chunkData, offset)) {
    reply(OTAErrorCode::FLASH_WRITE_ERROR);
    return;
  }

  session.receivedFlags[chunkIndex] = true;
  uint32_t received = static_cast<uint32_t>(std::count(
      session.receivedFlags.begin(), session.receivedFlags.end(), true));
  flashProgressPercent_ =
      static_cast<uint8_t>((received * 100) / session.totalChunks);
  session.lastActivity = std::chrono::steady_clock::now();
  reply(OTAErrorCode::SUCCESS);
}

// Helper: detect inactive partition
// Parses /proc/cmdline for root=<device>, returns the other one
std::string getInactivePartition() {
  std::ifstream cmdline("/proc/cmdline");
  std::string content;
  std::getline(cmdline, content);

  auto pos = content.find("root=/dev/");
  if (pos != std::string::npos) {
    pos += 10; // skip "root=/dev/"
    auto end = content.find(' ', pos);
    std::string rootDev = content.substr(pos, end - pos);

    if (rootDev == "mmcblk0p3")
      return "/dev/mmcblk0p2";
    if (rootDev == "mmcblk0p2")
      return "/dev/mmcblk0p3";

    throw std::runtime_error("Unrecognized root device: " + rootDev);
  }

  throw std::runtime_error("No root= found in /proc/cmdline");
}

// Helper: compute SHA256 of a file, returns lowercase hex string
std::string OTAFlashServiceStubImpl::computeSHA256(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return "";

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);

  char buf[65536];
  while (file.read(buf, sizeof(buf)) || file.gcount() > 0)
    EVP_DigestUpdate(ctx, buf, file.gcount());

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hashLen = 0;
  EVP_DigestFinal_ex(ctx, hash, &hashLen);
  EVP_MD_CTX_free(ctx);

  std::ostringstream oss;
  for (unsigned int i = 0; i < hashLen; i++)
    oss << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(hash[i]);

  return oss.str();
}

// finalizeTransfer
void OTAFlashServiceStubImpl::finalizeTransfer(
    const std::shared_ptr<CommonAPI::ClientId> /*_client*/,
    std::string transferId, finalizeTransferReply_t reply) {
  using OTAErrorCode = ::v1::com::ota::OTAFlashService::OTAErrorCode;
  using FlashState = ::v1::com::ota::OTAFlashService::FlashState;

  std::lock_guard<std::mutex> lock(sessionsMutex_);

  // LOok up session
  auto it = sessions_.find(transferId);
  if (it == sessions_.end()) {
    reply(OTAErrorCode::INVALID_TRANSFER_ID);
    return;
  }

  TransferSession &session = it->second;

  // Ensure all chunks arrived
  bool allReceived =
      std::all_of(session.receivedFlags.begin(), session.receivedFlags.end(),
                  [](bool f) { return f; });

  if (!allReceived) {
    reply(OTAErrorCode::INCOMPLETE_TRANSFER);
    return;
  }

  auto result = flashManager_.finalizeAndArm(session.expectedChecksum);

  if (result == FlashManager::FlashResult::ERR_CHECKSUM_MISMATCH) {
    reply(OTAErrorCode::CHECKSUM_MISMATCH);
    return;
  }
  if (result == FlashManager::FlashResult::ERR_UBOOT_ENV) {
    reply(OTAErrorCode::FLASH_WRITE_ERROR);
    return;
  }

  currentState_ = FlashState::COMPLETE;
  flashProgressPercent_ = 100;
  session.stagingPath = "direct"; // mark as finalized for startFlash guard

  reply(OTAErrorCode::SUCCESS);
}

void OTAFlashServiceStubImpl::watchdogWorker() {
  using OTAErrorCode = ::v1::com::ota::OTAFlashService::OTAErrorCode;
  using FlashState = ::v1::com::ota::OTAFlashService::FlashState;

  constexpr auto TIMEOUT = std::chrono::seconds(30);
  constexpr auto POLL = std::chrono::seconds(5);

  while (watchdogRunning_.load()) {
    std::this_thread::sleep_for(POLL);

    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto now = std::chrono::steady_clock::now();

    for (auto it = sessions_.begin(); it != sessions_.end();) {
      auto &session = it->second;
      bool finalized = !session.stagingPath.empty();

      if (!finalized && (now - session.lastActivity) > TIMEOUT) {
        // Clean up the time-out session
        std::string timeOutId = it->first;
        it = sessions_.erase(it);
        currentState_ = FlashState::FAILED;
        flashProgressPercent_ = 0;

        // Fire event so QNX proxy knows what happened
        fireFlashProgressEvent(timeOutId, FlashState::FAILED, 0,
                               OTAErrorCode::TRANSFER_TIMEOUT);
      } else {
        ++it;
      }
    }

    if (sessions_.empty()) {
      watchdogRunning_ = false;
    }
  }
}

// cancelTransfer
void OTAFlashServiceStubImpl::cancelTransfer(
    const std::shared_ptr<CommonAPI::ClientId> /*_client*/,
    std::string transferId, cancelTransferReply_t reply) {
  using OTAErrorCode = ::v1::com::ota::OTAFlashService::OTAErrorCode;
  using FlashState = ::v1::com::ota::OTAFlashService::FlashState;

  std::lock_guard<std::mutex> lock(sessionsMutex_);

  auto it = sessions_.find(transferId);
  if (it == sessions_.end()) {
    reply(OTAErrorCode::INVALID_TRANSFER_ID);
    return;
  }

  sessions_.erase(it);

  flashManager_ = FlashManager(); // reset flash state

  currentState_ = FlashState::IDLE;
  flashProgressPercent_ = 0;

  fireFlashProgressEvent(transferId, FlashState::IDLE, 0,
                         OTAErrorCode::SUCCESS);

  reply(OTAErrorCode::SUCCESS);
  return;
}