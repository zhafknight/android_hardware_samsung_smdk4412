#define LOG_TAG "exynos4-allocator5"

#include <aidl/android/hardware/graphics/allocator/BnAllocator.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <log/log.h>

namespace exynos4::gralloc5 {
std::shared_ptr<aidl::android::hardware::graphics::allocator::BnAllocator> createAllocator();
}

int main() {
    auto allocator = exynos4::gralloc5::createAllocator();
    const std::string name = std::string(allocator->descriptor) + "/default";
    ABinderProcess_setThreadPoolMaxThreadCount(4);
    if (AServiceManager_addService(allocator->asBinder().get(), name.c_str()) != STATUS_OK) {
        ALOGE("Failed to register %s", name.c_str());
        return 1;
    }
    ABinderProcess_joinThreadPool();
    return 0;
}
