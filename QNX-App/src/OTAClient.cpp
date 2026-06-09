#include "OTAClient.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <openssl/evp.h>
#include <sstream>
#include <thread>
#include <vector>

OTAClient::OTAClient() { m_runtime = CommonAPI::Runtime::get(); }

bool OTAClient::connect(const std::string &instance) {
  m_proxy = m_runtime->buildProxy<v1::com::ota::OTAFlashServiceProxy>(
      "local", instance, "OTAClient");
  if (!m_proxy)
    return false;

  // wait for service to be available
  auto start = std::chrono::steady_clock::now();
  while (!m_proxy->isAvailable()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (std::chrono::steady_clock::now() - start > std::chrono::seconds(10)) {
      std::cerr << "[QNX] Timeout waiting for OTA service\n";
      return false;
    }
  }
  std::cout << "[QNX] Connected to OTA service\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  return true;
}

bool OTAClient::sendImage(const std::string &imagePath) {
  std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
  if (!file)
    return false;

  uint64_t imageSize = file.tellg();
  uint32_t totalChunks = (imageSize + CHUNK_SIZE - 1) / CHUNK_SIZE;
  std::string checksum = computeSHA256(imagePath);

  file.seekg(0);

  CommonAPI::CallStatus callStatus;
  std::string transferId;
  v1::com::ota::OTAFlashService::OTAErrorCode errorCode;

  m_proxy->startTransfer(imageSize, totalChunks, checksum, callStatus,
                         transferId, errorCode);

  std::cerr << "[QNX] startTransfer callStatus: "
            << static_cast<int>(callStatus) << "\n";
  std::cerr << "[QNX] startTransfer errorCode: " << static_cast<int>(errorCode)
            << "\n";

  if (callStatus != CommonAPI::CallStatus::SUCCESS ||
      errorCode != v1::com::ota::OTAFlashService::OTAErrorCode::SUCCESS) {
    std::cerr << "[QNX] startTransfer failed: " << static_cast<int>(errorCode)
              << "\n";
    return false;
  }

  std::vector<uint8_t> buf(CHUNK_SIZE);
  for (uint32_t i = 0; i < totalChunks; ++i) {
    file.read(reinterpret_cast<char *>(buf.data()), CHUNK_SIZE);
    buf.resize(file.gcount());

    m_proxy->sendChunk(transferId, i, buf, callStatus, errorCode);
    if (callStatus != CommonAPI::CallStatus::SUCCESS ||
        errorCode != v1::com::ota::OTAFlashService::OTAErrorCode::SUCCESS) {
      std::cerr << "[QNX] Chunk " << i << " failed to send\n";
      return false;
    }

    std::cout << "\r[QNX] Sending... " << ((i + 1) * 100 / totalChunks) << "%"
              << std::flush;
    buf.resize(CHUNK_SIZE);
  }

  std::cout << "\n";

  CommonAPI::CallInfo callInfo(60000); // 60 seconds

  m_proxy->finalizeTransfer(transferId, callStatus, errorCode, &callInfo);

  if (callStatus == CommonAPI::CallStatus::SUCCESS &&
      errorCode == v1::com::ota::OTAFlashService::OTAErrorCode::SUCCESS) {
    std::cout << "[QNX] Image sent — RPi3 rebooting into new partition\n";
  } else {
    std::cout << "[QNX] Failed to transfer\n";
  }

  return true;
}

std::string OTAClient::computeSHA256(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return "";

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);

  char buf[4096];
  while (file.read(buf, sizeof(buf)) || file.gcount() > 0) {
    EVP_DigestUpdate(ctx, buf, file.gcount());
    if (!file)
      break;
  }

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hashLen = 0;
  EVP_DigestFinal_ex(ctx, hash, &hashLen);
  EVP_MD_CTX_free(ctx);

  std::ostringstream oss;
  for (unsigned int i = 0; i < hashLen; ++i)
    oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

  return oss.str();
}