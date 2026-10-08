/*
 * Shim for libexynoscamera3.so (Android O blob) on Android 14 libui:
 * - fd < 0 means "no fence" -> ready
 * - map sync_wait() ETIME to TIMED_OUT (blob compares against TIMED_OUT)
 * - on any other error log details and treat buffer as ready instead of
 *   dropping the frame (which shows up as green / frozen preview)
 */
#define LOG_TAG "CamFenceShim"

#include <errno.h>
#include <string.h>
#include <log/log.h>
#include <sync/sync.h>
#include <utils/Errors.h>

using android::status_t;

// android::Fence layout: LightRefBase<Fence> { atomic<int32_t> mCount; } + base::unique_fd mFenceFd
struct FenceLayout {
    int refCount;
    int fd;
};

extern "C" status_t _ZN7android5Fence4waitEi(void* self, int timeout) {
    const FenceLayout* f = static_cast<const FenceLayout*>(self);
    const int fd = f->fd;

    if (fd < 0) {
        return android::NO_ERROR;
    }

    if (sync_wait(fd, timeout) == 0) {
        return android::NO_ERROR;
    }

    const int err = errno;
    if (err == ETIME) {
        return android::TIMED_OUT;
    }

    ALOGW("Fence::wait(this=%p, fd=%d, timeout=%d) failed: %s (%d), treating as signaled",
          self, fd, timeout, strerror(err), err);
    return android::NO_ERROR;
}
