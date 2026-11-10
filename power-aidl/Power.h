/*
 * Minimal AIDL power HAL for universal8890 (herolte et al.).
 *
 * Its only job is to sleep the Samsung touchscreen (and touchkey) when the
 * display goes non-interactive, because nothing else does: the device cannot
 * fully suspend during a call (the CPU stays up for call audio), so the touch
 * driver's FB_BLANK suspend path never runs and the touchscreen stays live ->
 * accidental in-call taps. We write /sys/class/input/inputN/enabled directly.
 *
 * Pair this with config_powerDecoupleInteractiveModeFromDisplay=false so that
 * Mode::INTERACTIVE is coupled to the DISPLAY state and setMode(INTERACTIVE,
 * false) fires whenever the display turns off, including the in-call proximity
 * blank.
 */
#pragma once

#include <aidl/android/hardware/power/BnPower.h>

#include <string>
#include <vector>

namespace aidl::android::hardware::power::impl::universal8890 {

class Power : public BnPower {
  public:
    Power();

    ndk::ScopedAStatus setMode(Mode type, bool enabled) override;
    ndk::ScopedAStatus isModeSupported(Mode type, bool* _aidl_return) override;
    ndk::ScopedAStatus setBoost(Boost type, int32_t durationMs) override;
    ndk::ScopedAStatus isBoostSupported(Boost type, bool* _aidl_return) override;

  private:
    void findTouchInputNodes();

    // /sys/class/input/inputN/enabled paths for touch devices only
    // (sec_touchscreen, sec_touchkey) -- never gpio_keys/sensors.
    std::vector<std::string> mTouchEnabledNodes;
};

}  // namespace aidl::android::hardware::power::impl::universal8890
