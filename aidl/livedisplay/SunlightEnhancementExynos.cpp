/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/file.h>
#include <android-base/strings.h>
#include <livedisplay/samsung/SunlightEnhancementExynos.h>

#include <fstream>

using android::base::ReadFileToString;
using android::base::Trim;
using android::base::WriteStringToFile;

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {
namespace samsung {

static constexpr const char* kLUXPath = "/sys/class/mdnie/mdnie/lux";
static constexpr const char* kOutdoorPath = "/sys/class/mdnie/mdnie/outdoor";

static std::string GetSunlightPath() {
    std::fstream luxFile(kLUXPath, std::ios::in | std::ios::out);
    if (luxFile.good()) {
        return kLUXPath;
    }

    std::fstream outdoorFile(kOutdoorPath, std::ios::in | std::ios::out);
    if (outdoorFile.good()) {
        return kOutdoorPath;
    }

    return {};
}

bool SunlightEnhancementExynos::isSupported() {
    return !GetSunlightPath().empty();
}

// Methods from ::aidl::vendor::lineage::livedisplay::BnSunlightEnhancement follow.
ndk::ScopedAStatus SunlightEnhancementExynos::getEnabled(bool* _aidl_return) {
    const std::string path = GetSunlightPath();
    std::string tmp;
    int32_t contents = 0;

    if (path.empty() || !ReadFileToString(path, &tmp)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    contents = std::stoi(Trim(tmp));

    *_aidl_return = contents > 0;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SunlightEnhancementExynos::setEnabled(bool enabled) {
    const std::string path = GetSunlightPath();
    if (path.empty()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    // Newer Exynos kernels accept a lux value, while older kernels expose an outdoor toggle.
    const char* value = enabled ? (path == kLUXPath ? "40000" : "1") : "0";
    if (!WriteStringToFile(value, path, true)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace samsung
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
