#include "FlashManager.hpp"
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <linux/fs.h>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

FlashManager::FlashManager() = default;

bool FlashManager::openPartition() {
  std::string target = getInactivePartition();
  m_fd = open(target.c_str(), O_WRONLY | O_SYNC);
  return (m_fd >= 0);
}

void FlashManager::closePartition() {
  if (m_fd >= 0) {
    fsync(m_fd);
    close(m_fd);
    m_fd = -1;
  }
}

bool FlashManager::writeChunk(uint32_t chunkIndex,
                              const std::vector<uint8_t> &data,
                              uint32_t offset) {
  if (m_fd < 0 && !openPartition()) {
    return false;
  }

  // seek to the correct offset in the partition
  if (lseek(m_fd, offset, SEEK_SET) < 0) {
    return false;
  }

  ssize_t written = write(m_fd, data.data(), data.size());
  if (written != static_cast<ssize_t>(data.size())) {
    return false;
  }

  m_bytesWritten += data.size();
  // report progress if callback set and total size known
  if (m_progressCb && m_totalImageSize > 0) {
    uint8_t pct =
        static_cast<uint8_t>((m_bytesWritten * 100ULL) / m_totalImageSize);
    m_progressCb(pct);
  }
  return true;
}

FlashManager::FlashResult
FlashManager::finalizeAndArm(const std::string &expectedChecksum) {
  closePartition();

  // verify checksum by re-reading the partition
  std::string actual = computeSHA256();
  if (actual != expectedChecksum) {
    std::cerr << "[OTA] Checksum mismatch: expected=" << expectedChecksum
              << " actual=" << actual << "\n";
    return FlashResult::ERR_CHECKSUM_MISMATCH;
  }

  // arm uboot to boot rootfs_b next time
  if (!setBootPartition()) {
    std::cerr << "[OTA] setBootPartition failed\n";
    return FlashResult::ERR_UBOOT_ENV;
  }

  std::cerr << "[OTA] Boot partition armed successfully\n";
  sync();

  return FlashResult::SUCCESS;
}

std::string FlashManager::computeSHA256() {
  std::string target = getInactivePartition();
  int fd = open(target.c_str(), O_RDONLY);
  if (fd < 0) {
    std::cerr << "[OTA] Cannot open " << target << " for SHA256\n";
    return "";
  }

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);

  uint8_t buf[4096];
  uint64_t remaining = m_bytesWritten;
  while (remaining > 0) {
    size_t toRead = std::min(remaining, (uint64_t)sizeof(buf));
    ssize_t r = read(fd, buf, toRead);
    if (r <= 0)
      break;
    EVP_DigestUpdate(ctx, buf, r);
    remaining -= r;
  }
  close(fd);

  uint8_t hash[EVP_MAX_MD_SIZE];
  unsigned int hashLen = 0;
  EVP_DigestFinal_ex(ctx, hash, &hashLen);
  EVP_MD_CTX_free(ctx);

  std::ostringstream oss;
  for (unsigned int i = 0; i < hashLen; ++i)
    oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  return oss.str();
}

bool FlashManager::setBootPartition() {
  bool didMount = false;
  {
    std::ifstream mounts("/proc/mounts");
    std::string line;
    bool found = false;
    while (std::getline(mounts, line))
      if (line.find("/boot") != std::string::npos) {
        found = true;
        break;
      }
    if (!found) {
      if (system("mount /dev/mmcblk0p1 /boot") != 0) {
        std::cerr << "[OTA] Failed to mount /boot\n";
        return false;
      }
      didMount = true;
    }
  }

  std::string inactive = getInactivePartition();

  std::ifstream in("/boot/cmdline.txt");
  if (!in)
    return false;
  std::string content;
  std::getline(in, content);
  in.close();

  // replace root=/dev/mmcblkXpY with new partition
  auto pos = content.find("root=");
  if (pos == std::string::npos)
    return false;

  auto end = content.find(' ', pos);
  content.replace(pos, end - pos, "root=" + inactive);

  // also fix rootfstype if needed (ext3 vs ext4)
  auto fspos = content.find("rootfstype=ext4");
  if (fspos != std::string::npos)
    content.replace(fspos, 15, "rootfstype=ext3");

  std::ofstream out("/boot/cmdline.txt");
  if (!out) {
    std::cerr << "[OTA] Cannot write /boot/cmdline.txt\n";
    return false;
  }
  out << content; // single line, no trailing newline
  out.close();
  sync();

  if (didMount)
    system("umount /boot");

  return true;
}

void FlashManager::setProgressCallback(ProgressCallback cb) {
  m_progressCb = std::move(cb);
}

std::string FlashManager::getInactivePartition() {
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