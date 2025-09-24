#ifndef VERSION_INFO_H
#define VERSION_INFO_H

// Default version if not provided by build system
#ifndef VERSION_MAJOR
#define VERSION_MAJOR 1
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR 0
#endif

#ifndef VERSION_PATCH
#define VERSION_PATCH 0
#endif

#ifndef VERSION_BUILD
#define VERSION_BUILD __DATE__ " " __TIME__
#endif

#define VERSION_STRING_SIZE 64

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// Allow custom version string from build system
#ifndef VERSION_STRING
#define VERSION_STRING "MIDI2USB v" STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_PATCH)
#endif

extern const char version_info[];

#endif