#pragma once

#include <string>
#include <vector>
#include <set>

namespace GramExt {

class Extension;

class Controller {
public:
    static void initialize();
    static void enableExtension(Extension extension);
    static void disableExtension(Extension extension);

    static std::string transformOutputText(std::string text);
    static std::vector<Extension> getExtensions();
    static std::set<Extension> getEnabledExtensions();

private:
    static void reinitializeExtensions();
    static void jsPrint(void *v, void *);
    static void downloadExtension(std::string extensionURL);
    static std::string runScript(const std::string string);
    static std::vector<Extension> extensions;
};

}