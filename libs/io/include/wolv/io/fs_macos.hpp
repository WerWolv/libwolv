#pragma once

#if defined(OS_MACOS)

    extern "C" {
        char* getMacExecutableDirectoryPath();
    
        char* getMacMainBundleResourcesDirectoryPath();
        char* getMacMainBundleBuiltInPlugInsDirectoryPath();

        char* getMacApplicationSupportDirectoryPath();

        void macFree(void* ptr);
    }

#endif
