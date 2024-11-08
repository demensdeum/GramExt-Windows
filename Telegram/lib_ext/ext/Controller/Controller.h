#pragma once

#include <string>

namespace GramExt {

class Controller {
public:
    static void initialize();
    static std::string transformOutputText(std::string text);

private:
    static void jsPrint(void *v, void *);
    static std::string runScript(const std::string string);
};

}