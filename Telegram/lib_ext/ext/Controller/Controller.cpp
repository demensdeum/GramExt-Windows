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

using namespace GramExt;

CTinyJS *tinyJS = new CTinyJS();

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

void Controller::initialize() {
    registerFunctions(tinyJS);
    tinyJS->addNative("function print(text)", reinterpret_cast<JSCallback>(Controller::jsPrint), nullptr);
    tinyJS->addNative("function console.log(text)", reinterpret_cast<JSCallback>(Controller::jsPrint), nullptr);

    runScript(R"(
        print("JS Initialized 1!");
        console.log("JS Initialized 2!");
    )");
    runScript(listScript);
    runScript(sdkScript);

    std::string extensionsURLsRaw = downloadUrlToString("https://raw.githubusercontent.com/demensdeum/GramEXT-Extensions-List/refs/heads/main/extensions");
    std::vector<std::string> extensionsURLs = parseExtensionsURLs(extensionsURLsRaw);

    //std::string extensionsURLs[] = {
    //    "https://raw.githubusercontent.com/demensdeum/JokerText/refs/heads/main/extension/JokerTextMessageTransformer2.js",
    //    "https://raw.githubusercontent.com/demensdeum/JokerText/refs/heads/main/extension/main.js"
    //};

    for (std::string extensionURL : extensionsURLs) {
        std::string extension = downloadUrlToString(extensionURL);
        runScript(extension);
    }
}

std::string Controller::transformOutputText(const std::string text) {
    //return std::string(text) + "\n\nGramEXT_OutputTextTransformerTest";
    std::string js = "TransformOutputMessage(\"" + text +"\");";
    //std::string js = "TransformOutputMessage(\"\");";
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