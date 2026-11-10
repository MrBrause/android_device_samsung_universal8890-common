/*
 * Minimal AIDL power HAL for universal8890 -- touchscreen sleep on display off.
 * See Power.h for the rationale.
 */
#define LOG_TAG "android.hardware.power-service.universal8890"

#include "Power.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#include <dirent.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <memory>

namespace aidl::android::hardware::power::impl::universal8890 {

Power::Power() {
    findTouchInputNodes();
}

// Collect the "enabled" control node of every TOUCH input device. We match by
// the device's "name" containing "touch" (sec_touchscreen, sec_touchkey) so we
// never disable gpio_keys, the hall sensor, headset, etc. The touchscreen's
// enabled node reads back empty on this driver but is writable (0 disables, 1
// enables), so we key off the node existing, not its value.
void Power::findTouchInputNodes() {
    static const char kInputDir[] = "/sys/class/input/";

    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(kInputDir), &closedir);
    if (!dir) {
        LOG(ERROR) << "cannot scan " << kInputDir << ": " << strerror(errno);
        return;
    }

    struct dirent* ent;
    while ((ent = readdir(dir.get())) != nullptr) {
        // Only the per-device "inputN" entries carry a name/enabled pair.
        if (strncmp(ent->d_name, "input", 5) != 0) {
            continue;
        }

        const std::string base = std::string(kInputDir) + ent->d_name;

        std::string name;
        if (!::android::base::ReadFileToString(base + "/name", &name)) {
            continue;
        }
        name = ::android::base::Trim(name);
        if (name.find("touch") == std::string::npos) {
            continue;  // only sec_touchscreen / sec_touchkey
        }

        const std::string enabledPath = base + "/enabled";
        if (access(enabledPath.c_str(), F_OK) == 0) {
            LOG(INFO) << "managing touch input '" << name << "' via " << enabledPath;
            mTouchEnabledNodes.push_back(enabledPath);
        }
    }

    if (mTouchEnabledNodes.empty()) {
        LOG(WARNING) << "no touch 'enabled' nodes found; touchscreen will not be "
                        "slept on display off";
    }
}

ndk::ScopedAStatus Power::setMode(Mode type, bool enabled) {
    // With config_powerDecoupleInteractiveModeFromDisplay=false, INTERACTIVE is
    // coupled to the display: enabled==true -> display on, enabled==false ->
    // display off (including the in-call proximity blank). Mirror that onto the
    // touch controller.
    if (type == Mode::INTERACTIVE) {
        const std::string value = enabled ? "1" : "0";
        for (const auto& node : mTouchEnabledNodes) {
            if (!::android::base::WriteStringToFile(value, node)) {
                LOG(ERROR) << "failed writing " << value << " to " << node;
            }
        }
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isModeSupported(Mode type, bool* _aidl_return) {
    // We only act on INTERACTIVE; report just that as supported so the framework
    // keeps delivering it (it caches isModeSupported results).
    *_aidl_return = (type == Mode::INTERACTIVE);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::setBoost(Boost /*type*/, int32_t /*durationMs*/) {
    // No CPU boosting in this minimal HAL.
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isBoostSupported(Boost /*type*/, bool* _aidl_return) {
    *_aidl_return = false;
    return ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::power::impl::universal8890
