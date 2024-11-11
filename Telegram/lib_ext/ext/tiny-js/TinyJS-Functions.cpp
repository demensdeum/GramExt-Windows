/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * - Useful language functions
 *
 * Authored By Gordon Williams <gw@pur3.co.uk>
 *
 * Copyright (C) 2009 Pur3 Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 do
 * so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "TinyJS_Functions.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <sstream>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <cwctype>

using namespace std;
// ----------------------------------------------- Actual Functions
void scTrace(CScriptVar *, void *userdata) {
    CTinyJS *js = (CTinyJS *)userdata;
    js->root->trace();
}

void scObjectDump(CScriptVar *c, void *) {
    c->getParameter("this")->trace("> ");
}

void scObjectClone(CScriptVar *c, void *) {
    CScriptVar *obj = c->getParameter("this");
    c->getReturnVar()->copyValue(obj);
}

void scDateNow(CScriptVar *c, void *) {
    auto epoch = std::chrono::high_resolution_clock::now().time_since_epoch();
    double ms = static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count());
    // cout << ms << endl;
    c->getReturnVar()->setDouble(ms);
}

void scMathRand(CScriptVar *c, void *) {
    c->getReturnVar()->setDouble((double)rand() / RAND_MAX);
}

void scMathRandInt(CScriptVar *c, void *) {
    int min = c->getParameter("min")->getInt();
    int max = c->getParameter("max")->getInt();
    int val = min + (int)(rand() % (1 + max - min));
    c->getReturnVar()->setInt(val);
}

void scCharToInt(CScriptVar *c, void *) {
    string str = c->getParameter("ch")->getString();
    ;
    int val = 0;
    if (str.length() > 0)
        val = (int)str.c_str()[0];
    c->getReturnVar()->setInt(val);
}

void scStringIndexOf(CScriptVar *c, void *) {
    string str = c->getParameter("this")->getString();
    string search = c->getParameter("search")->getString();
    size_t p = str.find(search);
    int val = (p == string::npos) ? -1 : (int)p;
    c->getReturnVar()->setInt(static_cast<int>(val));
}

void scStringSubstring(CScriptVar *c, void *) {
    string str = c->getParameter("this")->getString();
    int lo = c->getParameter("lo")->getInt();
    int hi = c->getParameter("hi")->getInt();

    int l = hi - lo;
    if (l > 0 && lo >= 0 && lo + l <= (int)str.length())
        c->getReturnVar()->setString(str.substr((size_t)lo, (size_t)l));
    else
        c->getReturnVar()->setString("");
}

void scStringCharAt(CScriptVar* c, void*) {
    std::string str = c->getParameter("this")->getString();
    int pos = c->getParameter("pos")->getInt();

    std::string::iterator it = str.begin();
    int charPos = 0;

    while (it != str.end() && charPos < pos) {

        if ((*it & 0x80) == 0) {
            ++it;
        }
        else if ((*it & 0xE0) == 0xC0) {
            it += 2;
        }
        else if ((*it & 0xF0) == 0xE0) {
            it += 3;
        }
        else if ((*it & 0xF8) == 0xF0) {
            it += 4;
        }
        else {
            c->getReturnVar()->setString("");
            return;
        }
        ++charPos;
    }

    if (it != str.end()) {
        auto start = it;
        if ((*it & 0x80) == 0) {
            ++it;
        }
        else if ((*it & 0xE0) == 0xC0) {
            it += 2;
        }
        else if ((*it & 0xF0) == 0xE0) {
            it += 3;
        }
        else if ((*it & 0xF8) == 0xF0) {
            it += 4;
        }
        c->getReturnVar()->setString(std::string(start, it));
    }
    else {
        c->getReturnVar()->setString("");
    }
}

void scStringToUpperCase(CScriptVar* c, void*) {
    std::string str = c->getParameter("this")->getString();

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(str);


    for (auto& wc : wstr) {
        wc = std::towupper(wc);
    }

    str = converter.to_bytes(wstr);

    c->getReturnVar()->setString(str);
}

void scStringToLowerCase(CScriptVar* c, void*) {
    std::string str = c->getParameter("this")->getString();

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(str);

    for (auto& wc : wstr) {
        wc = std::towlower(wc);
    }

    str = converter.to_bytes(wstr);

    c->getReturnVar()->setString(str);
}

void scStringCharCodeAt(CScriptVar *c, void *) {
    string str = c->getParameter("this")->getString();
    int p = c->getParameter("pos")->getInt();
    if (p >= 0 && p < (int)str.length())
        c->getReturnVar()->setInt(str.at((size_t)p));
    else
        c->getReturnVar()->setInt(0);
}

void scStringSplit(CScriptVar *c, void *) {
    string str = c->getParameter("this")->getString();
    string sep = c->getParameter("separator")->getString();
    CScriptVar *result = c->getReturnVar();
    result->setArray();
    int length = 0;

    size_t pos = str.find(sep);
    while (pos != string::npos) {
        result->setArrayIndex(length++, new CScriptVar(str.substr(0, pos)));
        str = str.substr(pos + 1);
        pos = str.find(sep);
    }

    if (str.size() > 0)
        result->setArrayIndex(length++, new CScriptVar(str));
}

//void scStringFromCharCode(CScriptVar *c, void *) {
//    char str[2];
//    str[0] = static_cast<char>(c->getParameter("char")->getInt());
//    str[1] = 0;
//    c->getReturnVar()->setString(str);
//}

void scStringFromCharCode(CScriptVar* c, void*) {
    int unicodeValue = c->getParameter("char")->getInt();

    // Конвертация Unicode-символа в UTF-8 строку
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::string utf8str = converter.to_bytes(static_cast<char32_t>(unicodeValue));

    c->getReturnVar()->setString(utf8str.c_str());
}

void scIntegerParseInt(CScriptVar *c, void *) {
    string str = c->getParameter("str")->getString();
    size_t val = (size_t) strtol(str.c_str(), 0, 0);
    c->getReturnVar()->setInt(static_cast<int>(val));
}

void scDoubleParseDouble(CScriptVar *c, void *) {
    string str = c->getParameter("str")->getString();
    double val = strtod(str.c_str(), NULL);
    c->getReturnVar()->setDouble(val);
}

void scIntegerValueOf(CScriptVar *c, void *) {
    string str = c->getParameter("str")->getString();

    int val = 0;
    if (str.length() == 1)
        val = str[0];
    c->getReturnVar()->setInt(val);
}

void scJSONStringify(CScriptVar *c, void *) {
    std::ostringstream result;
    c->getParameter("obj")->getJSON(result);
    c->getReturnVar()->setString(result.str());
}

void scExec(CScriptVar *c, void *data) {
    CTinyJS *tinyJS = (CTinyJS *)data;
    std::string str = c->getParameter("jsCode")->getString();
    tinyJS->execute(str);
}

void scEval(CScriptVar *c, void *data) {
    CTinyJS *tinyJS = (CTinyJS *)data;
    std::string str = c->getParameter("jsCode")->getString();
    c->setReturnVar(tinyJS->evaluateComplex(str).var);
}

void scArrayContains(CScriptVar *c, void *) {
    CScriptVar *obj = c->getParameter("obj");
    CScriptVarLink *v = c->getParameter("this")->firstChild;

    bool contains = false;
    while (v) {
        if (v->var->equals(obj)) {
            contains = true;
            break;
        }
        v = v->nextSibling;
    }

    c->getReturnVar()->setInt(contains);
}

void scArrayRemove(CScriptVar *c, void *) {
    CScriptVar *obj = c->getParameter("obj");
    vector<int> removedIndices;
    CScriptVarLink *v;
    // remove
    v = c->getParameter("this")->firstChild;
    while (v) {
        if (v->var->equals(obj)) {
            removedIndices.push_back(v->getIntName());
        }
        v = v->nextSibling;
    }
    // renumber
    v = c->getParameter("this")->firstChild;
    while (v) {
        int n = v->getIntName();
        int newn = n;
        for (size_t i = 0; i < removedIndices.size(); i++)
            if (n >= removedIndices[i])
                newn--;
        if (newn != n)
            v->setIntName(newn);
        v = v->nextSibling;
    }
}

void scArrayJoin(CScriptVar *c, void *) {
    string sep = c->getParameter("separator")->getString();
    CScriptVar *arr = c->getParameter("this");

    ostringstream sstr;
    int l = arr->getArrayLength();
    for (int i = 0; i < l; i++) {
        if (i > 0)
            sstr << sep;
        sstr << arr->getArrayIndex(i)->getString();
    }

    c->getReturnVar()->setString(sstr.str());
}

// ----------------------------------------------- Register Functions
void registerFunctions(CTinyJS *tinyJS) {
    tinyJS->addNative("function exec(jsCode)", scExec,
                      tinyJS); // execute the given code
    tinyJS->addNative("function eval(jsCode)", scEval,
                      tinyJS); // execute the given string (an expression) and
                               // return the result
    tinyJS->addNative("function trace()", scTrace, tinyJS);
    tinyJS->addNative("function Object.dump()", scObjectDump, 0);
    tinyJS->addNative("function Object.clone()", scObjectClone, 0);
    tinyJS->addNative("function Math.rand()", scMathRand, 0);
    tinyJS->addNative("function Math.randInt(min, max)", scMathRandInt, 0);
    tinyJS->addNative("function charToInt(ch)", scCharToInt,
                      0); //  convert a character to an int - get its value
    tinyJS->addNative(
        "function String.indexOf(search)", scStringIndexOf,
        0); // find the position of a string in a string, -1 if not
    tinyJS->addNative("function String.toUpperCase()", scStringToUpperCase, 0);
    tinyJS->addNative("function String.toLowerCase()", scStringToLowerCase, 0);
    tinyJS->addNative("function String.substring(lo,hi)", scStringSubstring, 0);
    tinyJS->addNative("function String.charAt(pos)", scStringCharAt, 0);
    tinyJS->addNative("function String.charCodeAt(pos)", scStringCharCodeAt, 0);
    tinyJS->addNative("function String.fromCharCode(char)",
                      scStringFromCharCode, 0);
    tinyJS->addNative("function String.split(separator)", scStringSplit, 0);
    tinyJS->addNative("function Double.parseDouble(str)", scDoubleParseDouble,
                      0);
    tinyJS->addNative("function Integer.parseInt(str)", scIntegerParseInt,
                      0); // string to int
    tinyJS->addNative("function Integer.valueOf(str)", scIntegerValueOf,
                      0); // value of a single character
    tinyJS->addNative("function JSON.stringify(obj, replacer)", scJSONStringify,
                      0); // convert to JSON. replacer is ignored at the moment
    // JSON.parse is left out as you can (unsafely!) use eval instead
    tinyJS->addNative("function Array.contains(obj)", scArrayContains, 0);
    tinyJS->addNative("function Array.remove(obj)", scArrayRemove, 0);
    tinyJS->addNative("function Array.join(separator)", scArrayJoin, 0);
    tinyJS->addNative("function Date.now()", scDateNow, 0);
}