#include "common.h"
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <variant>


//fundamental json types:
//null, number(f64), string, array, dict
struct JSON : public std::variant<
    std::nullptr_t,
    f64, 
    std::string, 
    std::vector<JSON>, 
    std::unordered_map<std::string, JSON>> {
        using Map = std::unordered_map<std::string, JSON>;
        using String = std::string;
        using Array = std::vector<JSON>;
        using Number = f64;
        using Null = std::nullptr_t;
    };

JSON parse(const std::string& str, int& at, int e);

JSON parse_string(const std::string& str, int& at, int e) {
    std::string ret;
    assert(str[at] == '"');
    assert(at < e);
    at++;
    while (at <= e && str[at] != '"') {
        ret.push_back(str[at]);
        at++;
    }
    assert(str[at] == '"');
    return {ret};
}

JSON parse_array(const std::string& str, int& at, int e) {
    TimeFunction;
    std::vector<JSON> ret;
    assert(str[at] == '[');
    assert(at < e);
    at++;
    while (at <= e && str[at] != ']') {
        ret.push_back(parse(str, at, e));
        at++;
        if (str[at] == ']') break;
        assert(str[at] == ',');
        at++;
    }
    assert(str[at] == ']');
    return {ret};
}

JSON parse_dict(const std::string& str, int& at, int e) {
    std::unordered_map<std::string, JSON> ret;
    assert(str[at] == '{');
    assert(at < e);
    at++;
    while (at <= e && str[at] != '}') {
        auto key = parse_string(str, at, e);
        assert(str[at] == '"');
        at++;
        assert(str[at] == ':');
        at++;
        auto val = parse(str, at, e);
        at++;
        ret[std::get<std::string>(key)] = val;
        if (str[at] == '}') break;
        assert(str[at] == ',');
        at++;
    }
    assert(str[at] == '}');
    return {ret};
}

JSON parse_number(const std::string& str, int& at, int e) {
    assert(at <= e);
    assert(str[at] == '.' || str[at] == '-' || (str[at] >= '0' && str[at] <= '9'));
    const char* cstr = str.c_str();
    char* end;
    f64 ret = strtod(cstr+at, &end);
    at += (end - cstr - at - 1);
    return {ret};
}

JSON parse(const std::string& str, int& at, int e) {
    TimeFunction;
    if (at >= e) return {nullptr};
    JSON ret {nullptr};
    if (str[at] == '[') {
        ret = parse_array(str, at, e);
        assert(str[at] == ']');
    }
    else if (str[at] == '{') {
        ret = parse_dict(str, at, e);
        assert(str[at] == '}');
    }
    else if (str[at] == '"') {
        ret = parse_string(str, at, e);
        assert(str[at] == '"');
    }
    else if (str[at] == '.' || str[at] == '-' || (str[at] >= '0' && str[at] <= '9')) {
        ret = parse_number(str, at, e);
    }
    return ret;
}

JSON parseJSON(const std::string& str) {
    if (str.size() == 0) return {nullptr};
    int at = 0;
    return parse(str, at, str.size()-1);
}

// int main(int argc, char* argv[]) {

//     auto ret = parseJSON("[\"abc\",123,{\"key\":\"value\",\"key2\":[456,{\"key3\":[{\"key4\":{}}]}]}]");
//     auto& arr = std::get<std::vector<JSON>>(ret);
//     auto& obj = std::get<std::unordered_map<std::string, JSON>>(arr[2]);
//     auto& arr2 = std::get<std::vector<JSON>>(obj["key2"]);
//     std::cout << "hello";
//     return 0;
// }


