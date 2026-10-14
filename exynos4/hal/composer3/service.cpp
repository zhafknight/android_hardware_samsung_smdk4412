#define LOG_TAG "exynos4-composer3-service"

#include "ComposerClient.h"

#include <aidl/android/hardware/graphics/composer3/BnComposer.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <log/log.h>
#include <mutex>

namespace exynos4::composer3 {
class Composer final : public aidl::android::hardware::graphics::composer3::BnComposer {
  public:
    ndk::ScopedAStatus createClient(
            std::shared_ptr<aidl::android::hardware::graphics::composer3::IComposerClient>* out)
            override {
        std::lock_guard lock(mMutex);
        if (!mClient.expired())
            return ndk::ScopedAStatus::fromServiceSpecificError(
                    aidl::android::hardware::graphics::composer3::IComposerClient::EX_NO_RESOURCES);
        auto client = ndk::SharedRefBase::make<ComposerClient>();
        if (!client || !client->initialize())
            return ndk::ScopedAStatus::fromServiceSpecificError(
                    aidl::android::hardware::graphics::composer3::IComposerClient::EX_NO_RESOURCES);
        mClient = client;
        *out = std::move(client);
        return ndk::ScopedAStatus::ok();
    }

    ndk::ScopedAStatus getCapabilities(
            std::vector<aidl::android::hardware::graphics::composer3::Capability>* caps) override {
        caps->clear();
        return ndk::ScopedAStatus::ok();
    }

  private:
    std::mutex mMutex;
    std::weak_ptr<ComposerClient> mClient;
};
}  // namespace exynos4::composer3

int main() {
    auto composer = ndk::SharedRefBase::make<exynos4::composer3::Composer>();
    const std::string name = std::string(composer->descriptor) + "/default";
    ABinderProcess_setThreadPoolMaxThreadCount(4);
    ABinderProcess_startThreadPool();
    if (AServiceManager_addService(composer->asBinder().get(), name.c_str()) != STATUS_OK) {
        ALOGE("Cannot register %s", name.c_str());
        return 1;
    }
    ABinderProcess_joinThreadPool();
    return 0;
}
