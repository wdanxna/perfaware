
#include "common.h"
#include <fstream>

#include "metrics.cpp"

//The old haversine_compare.cpp is also 'instrumented', but for disambiguous purposes, this file
//is used to complete assignment for instrumentation utilities
#ifndef PROFILER
#define PROFILER 1
#endif

struct Profiler {
    struct Anchor {
        const char* name;
        u64 hit_count;
        u64 elapsed_inclusive;//containing all children's cost
        u64 elapsed_exclusive;//excluded all children's cost
        u64 bytes;//how many bytes does this block process
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
            printf("Total elapsed: %.2fms (%llu)\n", total_miliseconds, cpu_freq);
            for (int i = 1; i < ArrayCount(sections); i++) {
                auto& sec = sections[i];
                if (sec.elapsed_exclusive) {
                    //calculate data throughput
                    constexpr u64 Megabytes = 1024*1024;
                    constexpr u64 Gigabytes = 1024*Megabytes;
                    auto seconds = (f64)sec.elapsed_inclusive / cpu_freq;
                    auto amountMb = (f64)sec.bytes / Megabytes;//how many MB is processed in total
                    auto throughput = ((f64)sec.bytes / Gigabytes) / seconds; //how many GB is processed per second

                    auto ti = cpu_time_ms(sec.elapsed_inclusive);
                    auto te = cpu_time_ms(sec.elapsed_exclusive);
                    printf("    %s[%llu]: %.4fms", sec.name, sec.hit_count, te);
                    if (sec.elapsed_exclusive != sec.elapsed_inclusive) {
                        printf("(%.2f%%, %.2f%% inc)", 
                            (te/total_miliseconds)*100.0, 
                            (ti/total_miliseconds)*100.0);
                    } else {
                        printf("(%.2f%%)", (te/total_miliseconds)*100.0);
                    }
                    if (sec.bytes) {
                        printf(",(%.2fMB, %.4fGB/s)", amountMb, throughput);
                    }
                    printf("\n");
                }
                else break;
            }
        }
    }
};
static Profiler profiler;

#if PROFILER

static u64 globalParent;

struct scope_timer {
    u64 id;
    const char* name;
    u64 begin;
    u64 parent;
    u64 old_elapsed_inclusive;
    u64 bytes;
    scope_timer(const char* name, u64 bytes, u64 index) {
        this->name = name;
        this->id = index;
        this->bytes = bytes;
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
        sec.bytes += bytes;
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
#define TimeBlock(Name, Bytes) \
    scope_timer NameConcat(BLOCK, __LINE__)(Name, Bytes, __COUNTER__+1);
#define TimeFunction TimeBlock(__FUNCTION__, 0)
#define ProfilerValidation static_assert(__COUNTER__-1 < ArrayCount(profiler.sections));
#else
#define TimeBlock(...)
#define TimeFunction
#define ProfilerValidation
#endif

#include "json_parser.cpp"
#include "haversine_reference.cpp"

std::string read_file(const char* filename) {
    TimeFunction;

    u32 read = 0;
    char* buffer = new char[20*1024*1024];
    FILE *File = fopen(filename, "r");
    if (File)
    {
        TimeBlock("fread", 5358880);
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
    auto json = parseJSON(str);

    f64 sum = 0.0;
    {
        auto& map = std::get<JSON::Map>(json);
        assert(map.find("pairs") != map.end());
        auto& pairs = std::get<JSON::Array>(map["pairs"]);
        TimeBlock("compute", pairs.size() * sizeof(haversine_pair));
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

ProfilerValidation
