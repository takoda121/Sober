#pragma once

#include "types.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>

// vulkan needs at least 25 (7.1)
constexpr int ANDROID_SDK_VERSION = 26;

extern std::string roblox_version;

extern std::string roblox_locale;

extern std::string user_agent;
extern std::unordered_map<std::string, std::string> cookies;

extern usize display_width, display_height;
extern usize device_memory_mb;

struct UserState {
    i64 user_id;
    std::string username;
    std::string display_name;
    i32 membership_type;
    bool under_13;
    std::string country_code;
    std::string theme;
};

extern UserState user_state;


void set_path_prefix(std::filesystem::path prefix);
std::filesystem::path get_cookies_path();
std::filesystem::path get_assets_dir();
std::filesystem::path get_files_dir();
std::filesystem::path get_cache_dir();
std::filesystem::path get_core_lib_path();
std::string get_clientsettings_url();
