#define LOG_TAG "exynos4-allocator5"

#include <aidl/android/hardware/graphics/allocator/AllocationError.h>
#include <aidl/android/hardware/graphics/allocator/BnAllocator.h>
#include <aidl/android/hardware/graphics/common/BufferUsage.h>
#include <aidlcommonsupport/NativeHandle.h>
#include <cutils/native_handle.h>

#include "backend/BufferManager.h"
#include "gralloc_usage.h"

namespace exynos4::gralloc5 {
namespace {

using aidl::android::hardware::graphics::allocator::AllocationError;
using aidl::android::hardware::graphics::allocator::AllocationResult;
using aidl::android::hardware::graphics::allocator::BnAllocator;
using aidl::android::hardware::graphics::allocator::BufferDescriptorInfo;
using aidl::android::hardware::graphics::common::BufferUsage;

ndk::ScopedAStatus error(AllocationError code) {
    return ndk::ScopedAStatus::fromServiceSpecificError(static_cast<int32_t>(code));
}

bool supported(const BufferDescriptorInfo& d, BufferLayout* layout) {
    if (d.width <= 0 || d.height <= 0 || d.layerCount != 1 || d.reservedSize != 0 ||
        !d.additionalOptions.empty())
        return false;
    const uint64_t usage = static_cast<uint64_t>(d.usage);
    constexpr uint64_t kSupportedUsage =
            static_cast<uint64_t>(BufferUsage::CPU_READ_MASK) |
            static_cast<uint64_t>(BufferUsage::CPU_WRITE_MASK) |
            static_cast<uint64_t>(BufferUsage::GPU_TEXTURE) |
            static_cast<uint64_t>(BufferUsage::GPU_RENDER_TARGET) |
            static_cast<uint64_t>(BufferUsage::COMPOSER_OVERLAY) |
            static_cast<uint64_t>(BufferUsage::COMPOSER_CLIENT_TARGET) |
            static_cast<uint64_t>(BufferUsage::COMPOSER_CURSOR) |
            static_cast<uint64_t>(BufferUsage::VIDEO_ENCODER) |
            static_cast<uint64_t>(BufferUsage::CAMERA_OUTPUT) |
            static_cast<uint64_t>(BufferUsage::CAMERA_INPUT) |
            static_cast<uint64_t>(BufferUsage::VIDEO_DECODER) |
            GRALLOC_USAGE_EXYNOS4_SCANOUT |
            GRALLOC_USAGE_EXYNOS4_CONTIGUOUS;
    const uint64_t cpuRead = usage & 0xf;
    const uint64_t cpuWrite = (usage >> 4) & 0xf;
    // The Mali private handle serializes only the low 32 usage bits.
    if (usage > UINT32_MAX || (usage & ~kSupportedUsage) || cpuRead == 1 || cpuRead > 3 ||
        cpuWrite == 1 || cpuWrite > 3 || (usage & static_cast<uint64_t>(BufferUsage::PROTECTED)))
        return false;
    if (d.format == aidl::android::hardware::graphics::common::PixelFormat::BLOB &&
        (usage & (static_cast<uint64_t>(BufferUsage::GPU_TEXTURE) |
                  static_cast<uint64_t>(BufferUsage::GPU_RENDER_TARGET) |
                  static_cast<uint64_t>(BufferUsage::COMPOSER_OVERLAY) |
                  static_cast<uint64_t>(BufferUsage::COMPOSER_CLIENT_TARGET))))
        return false;
    return computeLayout(d.width, d.height, static_cast<int32_t>(d.format), layout);
}

class Allocator final : public BnAllocator {
  public:
    ndk::ScopedAStatus allocate(const std::vector<uint8_t>&, int32_t, AllocationResult*) override {
        return error(AllocationError::UNSUPPORTED);
    }

    ndk::ScopedAStatus allocate2(const BufferDescriptorInfo& d, int32_t count,
                                 AllocationResult* result) override {
        BufferLayout layout;
        if (count <= 0 || count > 64 || !supported(d, &layout))
            return error(AllocationError::BAD_DESCRIPTOR);
        result->stride = layout.stride;
        result->buffers.clear();
        result->buffers.reserve(count);
        for (int32_t i = 0; i < count; ++i) {
            native_handle_t* h = allocateBuffer(d.width, d.height, static_cast<int32_t>(d.format),
                                                static_cast<uint64_t>(d.usage), layout,
                                                reinterpret_cast<const char*>(d.name.data()));
            if (!h) {
                result->buffers.clear();
                return error(AllocationError::NO_RESOURCES);
            }
            // Transfer the ION fd to AIDL; the native-handle storage alone
            // is deleted, so neither side closes the other's descriptor.
            result->buffers.push_back(android::makeToAidl(h));
            native_handle_delete(h);
        }
        return ndk::ScopedAStatus::ok();
    }

    ndk::ScopedAStatus isSupported(const BufferDescriptorInfo& d, bool* out) override {
        BufferLayout layout;
        *out = supported(d, &layout);
        return ndk::ScopedAStatus::ok();
    }

    ndk::ScopedAStatus getIMapperLibrarySuffix(std::string* out) override {
        *out = "exynos4";
        return ndk::ScopedAStatus::ok();
    }
};

}  // namespace

std::shared_ptr<BnAllocator> createAllocator() {
    return ndk::SharedRefBase::make<Allocator>();
}

}  // namespace exynos4::gralloc5
