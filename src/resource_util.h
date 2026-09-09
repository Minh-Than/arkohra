#include "rini.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>   // For mkdir / _mkdir
#include <sys/types.h>

bool dir_exists(const char* path);
rini_data fetch_rini_config();
bool rini_key_exists(rini_data *data, const char *key);
int get_appdata_path(const char* appName, char* outPath, int maxLen);
void text_copy_bounded(char *dst, size_t dst_size, const char *src);

#ifdef _WIN32
    #include <direct.h>   // For _mkdir on Windows
    #define MKDIR(path) _mkdir(path)
#else
    #include <unistd.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

