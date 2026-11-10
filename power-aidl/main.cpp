#define LOG_TAG "android.hardware.power-service.universal8890"

#include "Power.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using ::aidl::android::hardware::power::impl::universal8890::Power;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    std::shared_ptr<Power> power = ndk::SharedRefBase::make<Power>();

    const std::string instance = std::string(Power::descriptor) + "/default";
    binder_status_t status =
            AServiceManager_addService(power->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK) << "failed to register " << instance;

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // not reached
}
