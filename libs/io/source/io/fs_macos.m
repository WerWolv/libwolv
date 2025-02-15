#if defined(OS_MACOS)

    #include <string.h>
    #include <stdlib.h>
    #include <Foundation/Foundation.h>

    char* getMacExecutableDirectoryPath(void) {
        @autoreleasepool {
            const char *pathString = [[[[[[NSBundle mainBundle] executableURL] URLByResolvingSymlinksInPath] URLByDeletingLastPathComponent] path] UTF8String];

            char *result = malloc(strlen(pathString) + 1);
            strcpy(result, pathString);

            return result;
        }
    }

    char* getMacExecutablePath(void) {
        @autoreleasepool {
            const char *pathString = [[[[[NSBundle mainBundle] executableURL] URLByResolvingSymlinksInPath] path] UTF8String];

            char *result = malloc(strlen(pathString) + 1);
            if (result) {
                strcpy(result, pathString);
            }

            return result;
        }
    }

    char* getMacApplicationSupportDirectoryPath(void) {
        @autoreleasepool {
            NSError* error = nil;
            NSURL* dirUrl = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory
                                                                   inDomain:NSUserDomainMask
                                                          appropriateForURL:nil
                                                                     create:YES
                                                                      error:&error];

            if (error != nil) {
                return NULL;
            }

            const char *pathString = [[dirUrl path] UTF8String];

            char *result = malloc(strlen(pathString) + 1);
            strcpy(result, pathString);

            return result;
        }
    }

    void macFree(void *ptr) {
        free(ptr);
    }

#endif
