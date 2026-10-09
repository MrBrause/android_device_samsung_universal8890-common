#include <cstddef>
extern "C" char* __cxa_demangle(const char* /*mangled*/, char* /*buf*/,
                                size_t* /*len*/, int* status) {
    if (status) *status = -2;   // "invalid mangled name" -> caller uses raw name
    return nullptr;
}
