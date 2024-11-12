#include "Controller.h"
#include <ext/tiny-js/TinyJS.h>
#include <ext/tiny-js/TinyJS_Functions.h>
#include <ext/tiny-js/TinyJS_MathFunctions.h>
#include "js_list.h"
#include "js_sdk.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <sstream>
#include "ext/Extension/Extension.h"
#include <regex>

using namespace GramExt;

std::set<Extension> enabledExtensions;
CTinyJS* tinyJS = nullptr;

std::vector<GramExt::Extension> GramExt::Controller::extensions;

std::string fileContent(const char* path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        exit(3);
    }

    std::string startFileContent;
    std::string line;

    while (std::getline(file, line)) {
        startFileContent += line;
        startFileContent += '\n';
    }

    file.close();

    return startFileContent;
}

size_t writeDataCallback(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

std::string downloadUrlToString(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("GramEXT Error: can't initialize CURL");
    }

    std::string result;

    std::string uniqueUrl = url + "?_=" + std::to_string(time(nullptr));

    curl_easy_setopt(curl, CURLOPT_URL, uniqueUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    headers = curl_slist_append(headers, "Pragma: no-cache");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        throw std::runtime_error("GramEXT Error CURL: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return result;
}

void Controller::jsPrint(void *v, void *) {
    CScriptVar *var = (CScriptVar *) v;
    std::cout << var->getParameter("text")->getString() << std::endl;
}

std::vector<std::string> parseExtensionsURLs(const std::string& extensionsURLsRaw) {
    std::vector<std::string> extensionsURLs;
    std::istringstream stream(extensionsURLsRaw);
    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty()) {
            extensionsURLs.push_back(line);
        }
    }

    return extensionsURLs;
}

std::string resolveManifestURL(const std::string& repoUrl) {
    std::regex repoRegex(R"(https://github\.com/([^/]+)/([^/]+))");
    std::smatch match;

    if (std::regex_match(repoUrl, match, repoRegex) && match.size() == 3) {
        std::string owner = match[1].str();
        std::string repo = match[2].str();

        return "https://raw.githubusercontent.com/" + owner + "/" + repo + "/refs/heads/main/manifest.json";
    }
    else {
        throw std::runtime_error("Invalid URL, can't resolve manifest.json");
    }
}

std::string resolveExecutableURL(const Extension& extension, const std::string& executableFilename) {
    std::string rawUrl = extension.rootUrl;

    std::string githubPrefix = "https://github.com/";
    std::string rawPrefix = "https://raw.githubusercontent.com/";

    if (rawUrl.find(githubPrefix) == 0) {
        rawUrl.replace(0, githubPrefix.length(), rawPrefix);
    }

    rawUrl += "/refs/heads/main/extension/" + executableFilename;

    return rawUrl;
}

std::vector<Extension> Controller::getExtensions() {
    return Controller::extensions;
}

std::set<Extension> Controller::getEnabledExtensions() {
    return enabledExtensions;
}

void Controller::enableExtension(Extension extension) {
    enabledExtensions.insert(extension);
    reinitializeExtensions();
}

void Controller::disableExtension(Extension extension) {
    enabledExtensions.erase(extension);
    reinitializeExtensions();
}

void Controller::downloadExtension(std::string extensionURL) {
    std::string manifestURL = resolveManifestURL(extensionURL);
    std::string manifestJSON = downloadUrlToString(manifestURL);
    Extension extension = Extension::from_json(extensionURL, manifestJSON);
    extensions.push_back(extension);
}

void Controller::initialize() {
    reinitializeExtensions();

    std::string extensionsURLsRaw = downloadUrlToString("https://raw.githubusercontent.com/demensdeum/GramEXT-Extensions-List/refs/heads/main/extensions");
    std::vector<std::string> extensionsURLs = parseExtensionsURLs(extensionsURLsRaw);

    for (std::string extensionURL : extensionsURLs) {
        downloadExtension(extensionURL);
    }
}

void Controller::reinitializeExtensions() {
    if (tinyJS) {
        delete tinyJS;
        tinyJS = nullptr;
    }
    tinyJS = new CTinyJS();
    registerFunctions(tinyJS);
    tinyJS->addNative("function print(text)", reinterpret_cast<JSCallback>(Controller::jsPrint), nullptr);
    tinyJS->addNative("function console.log(text)", reinterpret_cast<JSCallback>(Controller::jsPrint), nullptr);

    runScript(R"(
        print("JS Initialized 1!");
        console.log("JS Initialized 2!");
    )");
    runScript(listScript);
    runScript(sdkScript);

    for (Extension enabledExtension : enabledExtensions) {
        for (std::string executableFilename : enabledExtension.executables) {
            std::string executableURL = resolveExecutableURL(enabledExtension, executableFilename);
            std::string extensionScript = downloadUrlToString(executableURL);
            runScript(extensionScript);
        }
    }
}

std::string Controller::transformOutputText(const std::string text) {
    std::string js = "TransformOutputMessage(\"" + text +"\");";

    try {
        std::string output = runScript(js);
        return output;
    }
    catch(std::exception exception) {
        return "JS TRANSFORMER BUG: " + std::string(exception.what());
    }
    //return text;
}

std::string Controller::runScript(const std::string script) {
    std::string runScript = "var global_returnValue = \"\";" + script;
    std::cout << runScript << std::endl;
    tinyJS->execute(runScript.c_str());
    std::string output = tinyJS->evaluate("global_returnValue");
    std::cout << "global_returnValue: " << output << std::endl;
    return output;
}