#include <stdbool.h>
#include <sys/param.h>
#include "raylib.h"
#include "resource_util.h"

bool dir_exists(const char* path) {
  struct stat st;
  return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

bool rini_key_exists(rini_data *data, const char *key)
{
  for (unsigned int i = 0; i < data->count; i++) if (strcmp(key, data->entries[i].key) == 0) return true;
  return false;
}

rini_data fetch_rini_config()
{
  char app_dir [MAXPATHLEN];
  char ini_path[MAXPATHLEN];

  // Step 1: Get the app data directory path
  if (get_appdata_path("arckohra", app_dir, MAXPATHLEN) != 0) {
    printf("ERROR: Could not resolve app data path.\n");
    return (rini_data){ 0 };
  }

  // Step 2: Create the directory if it doesn't exist
  if (!dir_exists(app_dir)) {
    printf("Creating directory: %s\n", app_dir);
    if (MKDIR(app_dir) != 0) {
      printf("ERROR: Failed to create directory: %s\n", app_dir);
      return (rini_data){ 0 };
    }
  }

  // Build full path to the .ini file
  snprintf(ini_path, MAXPATHLEN, "%s/config.ini", app_dir);

  // Step 3: Load the config — this works even if the file doesn't exist!
  //         rini_load() returns an empty rini_data if the file is missing.
  //         You can also pass NULL to create a fresh empty config.
  rini_data config;
  bool file_existed = FileExists(ini_path);  // raylib's FileExists()

  if (file_existed) {
    printf("Loading existing config: %s\n", ini_path);
    config = rini_load(ini_path);
  } else {
    printf("No config found, creating new one.\n");
    config = rini_load(NULL);   // Create empty config object
    rini_save(config, ini_path);
  }
  return config;
}

int get_appdata_path(const char* appName, char* outPath, int maxLen) {
#if defined(_WIN32)
  char* appData = getenv("APPDATA");
  if (!appData) return -1;
  snprintf(outPath, maxLen, "%s\\%s", appData, appName);
#elif defined(__APPLE__)
  char* home = getenv("HOME");
  if (!home) return -1;
  snprintf(outPath, maxLen, "%s/Library/Application Support/%s", home, appName);
#else
  char* xdg = getenv("XDG_CONFIG_HOME");
  if (xdg && xdg[0]) {
      snprintf(outPath, maxLen, "%s/%s", xdg, appName);
  } else {
      char* home = getenv("HOME");
      if (!home) return -1;
      snprintf(outPath, maxLen, "%s/.config/%s", home, appName);
  }
#endif
  return 0;
}

void text_copy_bounded(char *dst, size_t dst_size, const char *src)
{
  if (dst == NULL || dst_size == 0) return;
  if (src == NULL) { dst[0] = '\0'; return; }
  snprintf(dst, dst_size, "%s", src);
}
