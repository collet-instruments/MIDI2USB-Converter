#include "version_info.h"

__attribute__((section(".version_info"))) __attribute__((used))
const char version_info[VERSION_STRING_SIZE] = VERSION_STRING " Built:" VERSION_BUILD;