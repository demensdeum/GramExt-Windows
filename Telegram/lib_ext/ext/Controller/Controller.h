#pragma once

#include <string>
#include <vector>

namespace GramExt {

class Extension;

class Controller {
public:
    static void initialize();
    static void reset();
    static void enableExtension(Extension extension);

    static std::string transformOutputText(std::string text);
    static std::vector<Extension> extensions;


private:
    static void jsPrint(void *v, void *);
    static void downloadExtension(std::string extensionURL);
    static std::string runScript(const std::string string);
};

}