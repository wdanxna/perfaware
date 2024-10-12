#include "common.h"
#include <fstream>


#include "json_parser.cpp"
#include "haversine_reference.cpp"

int main(int argc, char* argv[]) {
    u32 read = 0;
    char* buffer = new char[20*1024*1024];
    // const char* FileName = "data_10_uniform.json";
    const char* FileName = "data_100000_cluster.json";
    FILE *File = fopen(FileName, "r");
    if (File)
    {
        read = fread(buffer, 1, 20*1024*1024, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
        assert(false);
    }

    std::string json_str(buffer, read);
    int at = 0;
    auto json = parse(json_str, at, json_str.size()-1);

    auto& map = std::get<JSON::Map>(json);
    assert(map.find("pairs") != map.end());

    auto& pairs = std::get<JSON::Array>(map["pairs"]);

    std::vector<f64> haversines;
    f64 sum = 0.0;
    for (auto& pair : pairs) {
        auto& m = std::get<JSON::Map>(pair);
        auto x0 = std::get<JSON::Number>(m["X0"]);
        auto y0 = std::get<JSON::Number>(m["Y0"]);
        auto x1 = std::get<JSON::Number>(m["X1"]);
        auto y1 = std::get<JSON::Number>(m["Y1"]);
        auto hav = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
        sum += hav;
        haversines.push_back(hav);
    }
    auto average = sum / pairs.size();

    std::cout << "Pair count: " << pairs.size() << "\n";
    std::cout << "Expected sum: " << average << "\n";

    return 0;
}
