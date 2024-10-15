
#include "common.h"
#include <fstream>


#include "json_parser.cpp"
#include "haversine_reference.cpp"
#include "metrics.cpp"

//The old haversine_compare.cpp is also 'instrumented', but for disambiguous purposes, this file
//is used to complete assignment for instrumentation utilities

struct aggregate_profiler {
    struct section {
        std::string name;
        u64 begin, end, elapsed;

        void start() {
            begin = ReadCPUTimer();
        }
        void stop() {
            end = ReadCPUTimer();
            elapsed = end - begin;
        }
    };

    std::vector<section> sections;
    u64 total_elapsed = 0;
    u64 cpu_freq = 0;

    aggregate_profiler() {
        cpu_freq = EstimateCPUFreq();
    }

    void addSection(const section& sec) {
        total_elapsed += sec.elapsed;
        sections.push_back(std::move(sec));
    }

    f64 total_elapsed_ms() {
        return 1000.0 * ((f64)total_elapsed / (f64)cpu_freq);
    }

    inline f64 cpu_time_ms(section& sec) {
        return 1000.0 * ((f64)(sec.end - sec.begin)/(f64)cpu_freq);
    }

    void report() {
        auto total = total_elapsed_ms();
        printf("Total elapsed: %.2fms\n", total);
        for (auto& s : sections) {
            auto t = cpu_time_ms(s);
            printf("    %s: %.2fms (%.2f%%)\n", s.name.c_str(), t, (t/total)*100.0);
        }
    }
};

aggregate_profiler global_profiler;

struct scope_timer {
    aggregate_profiler::section sec{};
    scope_timer(const char* name) {
        sec.name = name;
        sec.start();
    }
    ~scope_timer() {
        sec.stop();
        global_profiler.addSection(std::move(sec));
    }
};


#define TimeFunction \
    scope_timer profiler_scoper(__FUNCTION__);

std::string read_file(const char* filename) {
    TimeFunction

    u32 read = 0;
    char* buffer = new char[20*1024*1024];
    FILE *File = fopen(filename, "r");
    if (File)
    {
        read = fread(buffer, 1, 20*1024*1024, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", filename);
        assert(false);
    }
    return std::string(buffer, read);
}

JSON parse_json(const std::string& json) {
    TimeFunction;
    int at = 0;
    return parse(json, at, json.size()-1);
}

void compute(JSON::Array& pairs, std::vector<f64>& haversines, f64& average) {
    TimeFunction;
    f64 sum = 0.0;
    {
        scope_timer timer("haversine loop");
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
    }
    average = sum / pairs.size();
}

int main(int argc, char* argv[]) {

    auto json_str = read_file("data_100000_cluster.json");
    
    auto json = parse_json(json_str);
    auto& map = std::get<JSON::Map>(json);
    assert(map.find("pairs") != map.end());
    auto& pairs = std::get<JSON::Array>(map["pairs"]);

    std::vector<f64> haversines;
    f64 average;
    compute(pairs, haversines, average);

    std::cout << "Pair count: " << pairs.size() << "\n";
    std::cout << "Expected sum: " << average << "\n";

    global_profiler.report();
    return 0;
}
