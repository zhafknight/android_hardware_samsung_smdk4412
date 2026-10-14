#pragma once

#include <aidl/android/hardware/graphics/composer3/BnComposerClient.h>
#include <aidl/android/hardware/graphics/composer3/Luts.h>
#include <aidl/android/hardware/graphics/common/BlendMode.h>
#include <aidl/android/hardware/graphics/common/Dataspace.h>
#include <aidl/android/hardware/graphics/common/FRect.h>
#include <aidl/android/hardware/graphics/common/Rect.h>
#include <android-base/unique_fd.h>
#include <cutils/native_handle.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
#include "DisplayBackend.h"

namespace exynos4::composer3 {
namespace common = aidl::android::hardware::graphics::common;
using namespace aidl::android::hardware::graphics::composer3;

class ComposerClient final : public BnComposerClient {
  public:
    ComposerClient();
    ~ComposerClient() override;
    bool initialize();
    ndk::ScopedAStatus createLayer(int64_t displayId, int32_t bufferSlotCount,
                                   int64_t* layer) override;
    ndk::ScopedAStatus createVirtualDisplay(int32_t width, int32_t height,
                                            common::PixelFormat formatHint,
                                            int32_t outputBufferSlotCount,
                                            VirtualDisplay* display) override;
    ndk::ScopedAStatus destroyLayer(int64_t displayId, int64_t layer) override;
    ndk::ScopedAStatus destroyVirtualDisplay(int64_t displayId) override;
    ndk::ScopedAStatus executeCommands(const std::vector<DisplayCommand>& commands,
                                       std::vector<CommandResultPayload>* results) override;
    ndk::ScopedAStatus getActiveConfig(int64_t displayId, int32_t* config) override;
    ndk::ScopedAStatus getColorModes(int64_t displayId,
                                     std::vector<ColorMode>* colorModes) override;
    ndk::ScopedAStatus getDataspaceSaturationMatrix(common::Dataspace dataspace,
                                                    std::vector<float>* matrix) override;
    ndk::ScopedAStatus getDisplayAttribute(int64_t displayId, int32_t config,
                                           DisplayAttribute attribute, int32_t* value) override;
    ndk::ScopedAStatus getDisplayCapabilities(int64_t displayId,
                                              std::vector<DisplayCapability>* caps) override;
    ndk::ScopedAStatus getDisplayConfigs(int64_t displayId, std::vector<int32_t>* configs) override;
    ndk::ScopedAStatus getDisplayConnectionType(int64_t displayId,
                                                DisplayConnectionType* type) override;
    ndk::ScopedAStatus getDisplayIdentificationData(int64_t displayId,
                                                    DisplayIdentification* id) override;
    ndk::ScopedAStatus getDisplayName(int64_t displayId, std::string* name) override;
    ndk::ScopedAStatus getDisplayVsyncPeriod(int64_t displayId, int32_t* vsyncPeriod) override;
    ndk::ScopedAStatus getDisplayedContentSample(int64_t displayId, int64_t maxFrames,
                                                 int64_t timestamp,
                                                 DisplayContentSample* samples) override;
    ndk::ScopedAStatus getDisplayedContentSamplingAttributes(
            int64_t displayId, DisplayContentSamplingAttributes* attrs) override;
    ndk::ScopedAStatus getDisplayPhysicalOrientation(int64_t displayId,
                                                     common::Transform* orientation) override;
    ndk::ScopedAStatus getHdrCapabilities(int64_t displayId, HdrCapabilities* caps) override;
    ndk::ScopedAStatus getOverlaySupport(OverlayProperties* properties) override;
    ndk::ScopedAStatus getMaxVirtualDisplayCount(int32_t* count) override;
    ndk::ScopedAStatus getPerFrameMetadataKeys(int64_t displayId,
                                               std::vector<PerFrameMetadataKey>* keys) override;
    ndk::ScopedAStatus getReadbackBufferAttributes(int64_t displayId,
                                                   ReadbackBufferAttributes* attrs) override;
    ndk::ScopedAStatus getReadbackBufferFence(int64_t displayId,
                                              ndk::ScopedFileDescriptor* acquireFence) override;
    ndk::ScopedAStatus getRenderIntents(int64_t displayId, ColorMode mode,
                                        std::vector<RenderIntent>* intents) override;
    ndk::ScopedAStatus getSupportedContentTypes(int64_t displayId,
                                                std::vector<ContentType>* types) override;
    ndk::ScopedAStatus getDisplayDecorationSupport(
            int64_t displayId, std::optional<common::DisplayDecorationSupport>* support) override;
    ndk::ScopedAStatus registerCallback(
            const std::shared_ptr<IComposerCallback>& callback) override;
    ndk::ScopedAStatus setActiveConfig(int64_t displayId, int32_t config) override;
    ndk::ScopedAStatus setActiveConfigWithConstraints(
            int64_t displayId, int32_t config, const VsyncPeriodChangeConstraints& constraints,
            VsyncPeriodChangeTimeline* timeline) override;
    ndk::ScopedAStatus setBootDisplayConfig(int64_t displayId, int32_t config) override;
    ndk::ScopedAStatus clearBootDisplayConfig(int64_t displayId) override;
    ndk::ScopedAStatus getPreferredBootDisplayConfig(int64_t displayId, int32_t* config) override;
    ndk::ScopedAStatus getHdrConversionCapabilities(
            std::vector<aidl::android::hardware::graphics::common::HdrConversionCapability>*)
            override;
    ndk::ScopedAStatus setHdrConversionStrategy(
            const aidl::android::hardware::graphics::common::HdrConversionStrategy&
                    conversionStrategy,
            aidl::android::hardware::graphics::common::Hdr* preferredHdrOutputType) override;
    ndk::ScopedAStatus setAutoLowLatencyMode(int64_t displayId, bool on) override;
    ndk::ScopedAStatus setClientTargetSlotCount(int64_t displayId, int32_t count) override;
    ndk::ScopedAStatus setColorMode(int64_t displayId, ColorMode mode,
                                    RenderIntent intent) override;
    ndk::ScopedAStatus setContentType(int64_t displayId, ContentType type) override;
    ndk::ScopedAStatus setDisplayedContentSamplingEnabled(int64_t displayId, bool enable,
                                                          FormatColorComponent componentMask,
                                                          int64_t maxFrames) override;
    ndk::ScopedAStatus setPowerMode(int64_t displayId, PowerMode mode) override;
    ndk::ScopedAStatus setReadbackBuffer(
            int64_t displayId, const aidl::android::hardware::common::NativeHandle& buffer,
            const ndk::ScopedFileDescriptor& releaseFence) override;
    ndk::ScopedAStatus setVsyncEnabled(int64_t displayId, bool enabled) override;
    ndk::ScopedAStatus setIdleTimerEnabled(int64_t displayId, int32_t timeoutMs) override;
    ndk::ScopedAStatus setRefreshRateChangedCallbackDebugEnabled(int64_t displayId,
                                                                 bool enabled) override;
    ndk::ScopedAStatus getDisplayConfigurations(int64_t displayId, int32_t maxFrameIntervalNs,
                                                std::vector<DisplayConfiguration>*) override;
    ndk::ScopedAStatus notifyExpectedPresent(int64_t displayId,
                                             const ClockMonotonicTimestamp& expectedPresentTime,
                                             int32_t maxFrameIntervalNs) override;
    ndk::ScopedAStatus getMaxLayerPictureProfiles(int64_t displayId,
                                                  int32_t* outMaxProfiles) override;
    ndk::ScopedAStatus startHdcpNegotiation(
            int64_t displayId, const aidl::android::hardware::drm::HdcpLevels& levels) override;
    ndk::ScopedAStatus getLuts(int64_t displayId, const std::vector<Buffer>&,
                               std::vector<Luts>*) override;

  private:
    struct BufferSlot {
        native_handle_t* handle = nullptr;
        bool cached = false;
    };
    struct Layer {
        Composition composition = Composition::CLIENT;
        int32_t z = 0;
        int32_t slot = -1;
        std::vector<BufferSlot> slots;
        android::base::unique_fd acquireFence;
        common::Rect displayFrame{};
        common::FRect sourceCrop{};
        common::BlendMode blendMode = common::BlendMode::NONE;
        common::Transform transform = common::Transform::NONE;
        common::Dataspace dataspace = common::Dataspace::UNKNOWN;
        float planeAlpha = 1.0f;
        uint32_t color = 0;
        float colorAlpha = 1.0f;
        bool hasDisplayFrame = false;
        bool hasSourceCrop = false;
        bool hasColorTransform = false;
        bool hasLuts = false;
        bool hasBrightness = false;
        bool hasHdrMetadata = false;
        bool hasPictureProfile = false;
    };
    struct RetiredHandle {
        native_handle_t* handle = nullptr;
        android::base::unique_fd fence;
    };
    struct FimcScratch {
        native_handle_t* handle = nullptr;
        android::base::unique_fd retireFence;
        bool displayed = false;
    };
    void clearLayer(Layer& layer);
    void collectRetiredHandles();
    bool isVideoLayer(const Layer& layer) const;
    native_handle_t* acquireFimcScratch(uint32_t width, uint32_t height);
    bool canScanout(const Layer& layer, bool bottom) const;
    void validate(std::vector<CommandResultPayload>* results);
    int present(std::vector<CommandResultPayload>* results);
    void vsyncLoop();
    DisplayBackend mDisplay;
    std::mutex mMutex;
    std::mutex mVsyncStateMutex;
    std::mutex mCallbackMutex;
    std::shared_ptr<IComposerCallback> mCallback;
    std::map<int64_t, Layer> mLayers;
    std::set<int64_t> mDisplayedDeviceLayers;
    std::vector<native_handle_t*> mPendingRetire;
    std::vector<RetiredHandle> mRetiredHandles;
    std::vector<FimcScratch> mFimcScratch;
    native_handle_t* mDisplayedFimcScratch = nullptr;
    std::vector<int64_t> mDeviceLayers;
    std::map<int64_t, Composition> mPendingChanges;
    bool mNeedsClientTarget = true;
    size_t mClientTargetInsertIndex = 0;
    bool mHasDisplayColorTransform = false;
    bool mHasDisplayPictureProfile = false;
    bool mDisableDeviceComposition = false;
    std::vector<native_handle_t*> mTargets;
    android::base::unique_fd mTargetAcquireFence;
    android::base::unique_fd mVsyncFd;
    common::Transform mOrientation = common::Transform::NONE;
    int32_t mTargetSlot = -1;
    bool mClientTargetUpdated = false;
    int64_t mNextLayer = 1;
    uint64_t mStateGeneration = 0;
    uint64_t mValidatedGeneration = 0;
    std::chrono::steady_clock::time_point mLastBadParameterLog{};
    bool mValidated = false;
    bool mVsyncEnabled = false;
    bool mPowerOn = false;
    std::atomic<bool> mStopVsync{false};
    std::thread mVsyncThread;
};
}  // namespace exynos4::composer3
