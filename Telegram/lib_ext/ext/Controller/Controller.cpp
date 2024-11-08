#include "Controller.h"
#include <ext/tiny-js/TinyJS.h>
#include "js_list.h"
#include <iostream>
#include <curl/curl.h>

size_t writeDataCallback(void* contents, size_t size, size_t nmemb, std::string* userData) {
    size_t totalSize = size * nmemb;
    userData->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string downloadUrlToString(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("GramEXT Error: can't initialize CURL");
    }

    std::string result;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("GramEXT Error CURL: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);
    return result;
}

using namespace GramExt;

CTinyJS *tinyJS = new CTinyJS();



const std::string controllerScript = R"(
    var outputMessageTransformers = new List();
    var OutputMessageAddTransformer = function(transformer) {
        outputMessageTransformers.push(transformer);
    };
    var TransformOutputMessage = function(message) {
        var output = message;
        var transformFunc = function(transformer) {
            output = transformer(output);
        };
        outputMessageTransformers.forEach(transformFunc);
        print(output);
        global_returnValue = output;
    };
)";

void Controller::jsPrint(void *v, void *) {
    CScriptVar *var = (CScriptVar *) v;
    std::cout << var->getParameter("text")->getString() << std::endl;
}

void Controller::initialize() {
    tinyJS->addNative("function print(text)", reinterpret_cast<JSCallback>(Controller::jsPrint), nullptr);
    tinyJS->addNative("function console.log(text)", reinterpret_cast<JSCallback>(Controller::jsPrint), nullptr);
    runScript(R"(
        print("JS Initialized 1!");
        console.log("JS Initialized 2!");
    )");
    runScript(listScript);
    runScript(controllerScript);

    std::string extensionsURLs[] = {
        "https://raw.githubusercontent.com/demensdeum/SignMessage/refs/heads/main/extension/main.js"
    };

    downloadUrlToString("http://demensdeum.com/blog/wp-content/uploads/2024/10/mediaradio-maximum-electron.png");

    for (std::string extensionURL : extensionsURLs) {
        std::string extension = downloadUrlToString(extensionURL);
        runScript(extension);
    }
}

std::string Controller::transformOutputText(const std::string text) {
    //return std::string(text) + "\n\nGramEXT_OutputTextTransformerTest";
    std::string output = runScript("TransformOutputMessage(\"" + text + "\");");
    return output;
}

std::string Controller::runScript(const std::string script) {
    std::string runScript = "var global_returnValue = \"\";" + script;
    std::cout << runScript << std::endl;
    tinyJS->execute(runScript.c_str());
    std::string output = tinyJS->evaluate("global_returnValue");
    std::cout << "global_returnValue: " << output << std::endl;
    return output;
}