#pragma once

#include <glaze/json.hpp>
#include <glaze/yaml.hpp>
#include <glaze/toml.hpp>
#include <string>

template <typename T>
[[nodiscard]]
std::string bindJson(const T& value) {
    std::string buffer{};
    
    if (auto ec = glz::write_json(value, buffer); ec) {
        log(LogType::Warn, "Couldn't parse json'");
        return "";
    }

    return buffer;
}

template <typename T>
[[nodiscard]]
std::string bindYaml(const T& value) {
    std::string buffer{};

    if (auto ec = glz::write_yaml(value, buffer); ec) {
        log(LogType::Warn, "Couldn't parse json'");
        return "";
    }

    return buffer;
}

template <typename T>
[[nodiscard]]
std::string bindToml(const T& value) {
    std::string buffer{};

    if (auto ec = glz::write_yaml(value, buffer); ec) {
        log(LogType::Warn, "Couldn't parse json'");
        return "";
    }

    return buffer;
}