
#include "common.h"
#include <fstream>

#include "metrics.cpp"

//The old haversine_compare.cpp is also 'instrumented', but for disambiguous purposes, this file
//is used to complete assignment for instrumentation utilities

struct Profiler {
    struct Anchor {
        const char* name;
        u64 hit_count;
        u64 elapsed_inclusive;//containing all children's cost
        u64 elapsed_exclusive;//excluded all children's cost
    };

    Anchor sections[4096];
    u64 cpu_freq = 0;
    u64 begin = 0, end = 0;

    inline f64 cpu_time_ms(u64 time) {
        return 1000.0 * ((f64)time/(f64)cpu_freq);
    }

    void beginProfiling() {
        begin = ReadCPUTimer();
    }

    void endProfiling() {
        end = ReadCPUTimer();
        cpu_freq = EstimateCPUFreq();
    }

    void report() {
        if (cpu_freq) {
            auto total_elapsed = end - begin;
            f64 total_miliseconds = 1000.0 * (f64)total_elapsed/(f64)cpu_freq;
            printf("Total elapsed: %.2fms\n", total_miliseconds);
            for (int i = 0; i < ArrayCount(sections); i++) {
                auto& sec = sections[i];
                if (sec.elapsed_exclusive) {
                    auto ti = cpu_time_ms(sec.elapsed_inclusive);
                    auto te = cpu_time_ms(sec.elapsed_exclusive);
                    printf("    %s[%llu]: %.4fms (%.4f%% exclusive, %.4f%% inclusive)\n", sec.name, sec.hit_count, te, (te/total_miliseconds)*100.0, (ti/total_miliseconds)*100.0);
                }
            }
        }
    }
};

static Profiler profiler;
static u64 globalParent;

struct scope_timer {
    u64 id;
    const char* name;
    u64 begin;
    u64 parent;
    u64 old_elapsed_inclusive;
    scope_timer(const char* name, u64 index) {
        this->name = name;
        this->id = index;
        begin = ReadCPUTimer();
        parent = globalParent;
        globalParent = index;

        auto& sec = profiler.sections[id];
        old_elapsed_inclusive = sec.elapsed_inclusive;
    }
    ~scope_timer() {
        u64 elapsed = ReadCPUTimer() - begin;
        auto& sec = profiler.sections[id];
        sec.hit_count++;
        //outmost call will overwrite all recursive calls' value, thus achieve the same effect of a nested counter
        //without a write to anchor object everytime the block opens.
        sec.elapsed_inclusive = old_elapsed_inclusive + elapsed;
        sec.elapsed_exclusive += elapsed;
        sec.name = name;

        //accumulate to parent
        if (parent != 0) {
            //0 is reserved
            auto& p = profiler.sections[parent];
            p.elapsed_exclusive -= elapsed;
        }
        globalParent = parent;
    }
};



#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) \
    scope_timer NameConcat(BLOCK, __LINE__)(Name, __COUNTER__+1);
#define TimeFunction TimeBlock(__FUNCTION__)

#include "json_parser.cpp"
#include "haversine_reference.cpp"

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

void parse_haversine_pairs(const std::string& str, std::vector<f64>& haversines, f64& average) {
    TimeFunction;

    int at = 0;
    auto json = parse(str, at, str.size()-1);

    f64 sum = 0.0;
    {
        TimeBlock("lookup and convert");
        auto& map = std::get<JSON::Map>(json);
        assert(map.find("pairs") != map.end());
        auto& pairs = std::get<JSON::Array>(map["pairs"]);
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
        average = sum / pairs.size();
    }
}

int main(int argc, char* argv[]) {

    profiler.beginProfiling();

    auto json_str = read_file("data_100000_cluster.json");
    
    std::vector<f64> haversines;
    f64 average;
    parse_haversine_pairs(json_str, haversines, average);


    std::cout << "Pair count: " << haversines.size() << "\n";
    std::cout << "Expected sum: " << average << "\n";

    profiler.endProfiling();
    profiler.report();
    return 0;
}

static_assert(__COUNTER__-1 < ArrayCount(profiler.sections));
