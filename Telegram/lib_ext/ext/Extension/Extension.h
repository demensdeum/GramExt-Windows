#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <atomic>
#include <set>

namespace GramExt {

    class Extension {

    public:
        const int id;
        const std::string rootUrl;
        const std::string sdkVersion;
        const std::string runtime;
        const std::string title;
        const std::string info;
        const std::string buildVersion;
        const std::string license;
        const std::vector<std::string> access;
        const std::vector<std::string> executables;

        static std::atomic<int> nextId;

        Extension(const std::string rootUrl, const nlohmann::json& j)
            : id(nextId.fetch_add(1)),
            rootUrl(rootUrl),
            sdkVersion(j.at("sdkVersion").get<std::string>()),
            runtime(j.at("runtime").get<std::string>()),
            title(j.at("title").get<std::string>()),
            info(j.at("info").get<std::string>()),
            buildVersion(j.at("buildVersion").get<std::string>()),
            license(j.at("license").get<std::string>()),
            access(j.at("access").get<std::vector<std::string>>()),
            executables(j.at("executables").get<std::vector<std::string>>()) {}

        bool operator<(const Extension& other) const {
            return id < other.id;
        }

        static Extension from_json(const std::string rootUrl, const std::string& json_str) {
            nlohmann::json j = nlohmann::json::parse(json_str);
            return Extension(rootUrl, j);
        }
    };

}
