/*
 * libexynoscamera3.so (Android O blob) allocates framework objects with the
 * Android O sizeof, e.g. GraphicBuffer: 144 bytes, while the Android 14
 * constructor needs 160 bytes. The blob's operator new import is renamed to
 * camnw() (see extract-files.sh), which pads every allocation so newer,
 * larger layouts fit. Freed normally via operator delete -> free().
 */
#include <stdlib.h>

static constexpr size_t kBlobAllocPad = 256;

extern "C" void* camnw(size_t size) {
    void* p = malloc(size + kBlobAllocPad);
    if (p == nullptr) {
        abort();  // operator new must never return nullptr
    }
    return p;
}
