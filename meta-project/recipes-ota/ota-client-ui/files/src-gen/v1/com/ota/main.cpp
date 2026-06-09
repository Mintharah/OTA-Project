#include <CommonAPI/CommonAPI.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "OTAFlashServiceStubImpl.hpp"

int main(int argc, char** argv) {
  //picks up vsomeip config
  std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::get();

  const std::string domain = "local";
  const std::string instance = "com.ota.OTAFlashService";
  const std::string connection = "OTAFlashService";

  auto service = std::make_shared<OTAFlashServiceStubImpl>();

  bool registered = runtime->registerService(domain, instance, service, connection);

  if(!registered)
{
  std::cerr << "[OTA] Failed to register service\n";
}  
  std::cout << "[OTA] Service registered - waiting for QNX client...\n";

  while(true){
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  runtime->unregisterService(domain, OTAFlashServiceStubImpl::StubInterface::getInterface(), instance);
return 0;
}