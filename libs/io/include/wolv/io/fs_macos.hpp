#pragma once

#if defined(OS_MACOS)

    extern "C" char* getMacExecutableDirectoryPath();
    extern "C" char* getMacExecutablePath();
    extern "C" char* getMacApplicationSupportDirectoryPath();

    extern "C" void macFree(void *ptr);

#endif
