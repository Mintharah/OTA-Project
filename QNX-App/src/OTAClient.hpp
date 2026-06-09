#ifndef OTA_CLIENT_HPP
#define OTA_CLIENT_HPP

#include <CommonAPI/CommonAPI.hpp>
#include <string>
#include "v1/com/ota/OTAFlashServiceProxy.hpp"


using OTAProxy = v1::com::ota::OTAFlashServiceProxy<>;

class OTAClient {
public:
  OTAClient();
  bool connect(const std::string &instance = "com.ota.OTAFlashService");
  bool sendImage(const std::string &imagePath);

private:
  std::shared_ptr<CommonAPI::Runtime> m_runtime;
  std::shared_ptr<OTAProxy> m_proxy;

  static constexpr uint32_t CHUNK_SIZE = 65536; // max allowed
  std::string computeSHA256(const std::string &path);
};

#endif // OTA_CLIENT_HPP