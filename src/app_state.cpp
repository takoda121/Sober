#include "app_state.hpp"
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>


std::string roblox_version = "";
std::string user_agent = "Roblox/WinInet"; // makes roblox think our client is a computer (well it actually is but yea)

std::optional<std::filesystem::path> path_prefix = std::nullopt;

std::unordered_map<std::string, std::string> cookies;

usize display_width = 0, display_height = 0;
usize device_memory_mb = 0;

UserState user_state = {
    .user_id = -1,
    .username = "",
    .display_name = "",
    .membership_type = 0,
    .under_13 = false,
    .country_code = "",
    .theme = "Dark",
};


static std::filesystem::path get_data_home() {
    const char *data_home = std::getenv("XDG_DATA_HOME");
    if (data_home) {
        return std::filesystem::path(data_home) / "sober";
    }

    const char *home_dir = std::getenv("HOME");
    if (!home_dir) {
        std::cerr << "HOME env variable not set" << std::endl;
        std::terminate();
    }

    return std::filesystem::path(home_dir) / ".local" / "share" / "sober";
}

static std::filesystem::path get_cache_home() {
    const char *cache_home = std::getenv("XDG_CACHE_HOME");
    if (cache_home) {
        return std::filesystem::path(cache_home) / "sober";
    }

    const char *home_dir = std::getenv("HOME");
    if (!home_dir) {
        std::cerr << "HOME env variable not set" << std::endl;
        std::terminate();
    }

    return std::filesystem::path(home_dir) / ".cache" / "sober";
}


void set_path_prefix(std::filesystem::path prefix) {
    path_prefix = prefix;
}

std::filesystem::path get_cookies_path() {
    if (path_prefix) {
        return *path_prefix / "cookies";
    }

    return get_data_home() / "cookies";
}

std::filesystem::path get_assets_dir() {
    if (path_prefix) {
        return *path_prefix / "assets/";
    }

    return get_data_home() / "assets/";
}

std::filesystem::path get_files_dir() {
    if (path_prefix) {
        return *path_prefix / "files/";
    }

    return get_data_home();
}

std::filesystem::path get_cache_dir() {
    if (path_prefix) {
        return *path_prefix / "cache/";
    }

    return get_cache_home();
}

std::filesystem::path get_core_lib_path() {
    if (path_prefix) {
        return *path_prefix / "libroblox.so";
    }

    return get_data_home() / "libroblox.so";
}

std::string get_clientsettings_url() {
    return "https://clientsettingscdn.roblox.com/v2/settings/application/GoogleAndroidApp";
}
