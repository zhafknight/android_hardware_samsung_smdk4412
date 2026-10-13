/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "CamComm1.0-SamsungCamModule"
#define ATRACE_TAG ATRACE_TAG_CAMERA

#include <SamsungCameraModule.h>
#include <cutils/trace.h>

#include <cstdint>
#include <cstring>

namespace android {
namespace hardware {
namespace camera {
namespace common {
namespace helper {

SamsungCameraModule::SamsungCameraModule(camera_module_t* module)
    : CameraModule(module), mModule(module) {}

SamsungCameraModule::~SamsungCameraModule() {}

namespace {

using SetTorchModeStrength = int (*)(const char*, bool, int32_t);

SetTorchModeStrength getSetTorchModeStrength(camera_module_t* module) {
    if (module == nullptr) {
        return nullptr;
    }

    // CameraModule.h includes the platform camera.h first, so camera_module_t
    // has the platform layout in this translation unit. Samsung's extension
    // inserts set_torch_mode_strength at the address of platform reserved[0].
    static_assert(sizeof(module->reserved[0]) == sizeof(SetTorchModeStrength));
    SetTorchModeStrength function = nullptr;
    memcpy(&function, &module->reserved[0], sizeof(function));
    return function;
}

}  // namespace

bool SamsungCameraModule::isSetTorchModeStrengthSupported() {
    return isSetTorchModeSupported() && getSetTorchModeStrength(mModule) != nullptr;
}

int SamsungCameraModule::setTorchModeStrength(const char* camera_id, bool enable, int strength) {
    int res = INVALID_OPERATION;
    const SetTorchModeStrength setTorchModeStrength = getSetTorchModeStrength(mModule);
    if (setTorchModeStrength != nullptr) {
        ATRACE_BEGIN("camera_module->set_torch_mode_strength");
        res = setTorchModeStrength(camera_id, enable, strength);
        ATRACE_END();
    }
    return res;
}

}  // namespace helper
}  // namespace common
}  // namespace camera
}  // namespace hardware
}  // namespace android
