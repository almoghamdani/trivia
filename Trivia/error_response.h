#pragma once
#include <string>

#include "json.hpp"
using nlohmann::json;

struct ErrorResponse {
    std::string message;
};

void to_json(json& j, const ErrorResponse& res) {
    j = json{ {"message", res.message} };
}

void from_json(const json& j, ErrorResponse& res) {
    j.at("message").get_to(res.message);
}