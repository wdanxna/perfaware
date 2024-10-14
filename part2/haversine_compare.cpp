#include "common.h"
#include <fstream>


#include "json_parser.cpp"
#include "haversine_reference.cpp"
#include "metrics.cpp"

//this file can only be executed with root privilege
 
int main(int argc, char* argv[]) {
    u64 CPUFreq = EstimateCPUFreq();
    u64 OSFreq = GetOSTimerFreq();

    u64 OSTotalStart = ReadOSTimer();
    u64 CPUTotalStart = ReadCPUTimer();


        u64 CPUReadFileStart = ReadCPUTimer();
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
        u64 CPUReadFileEnd = ReadCPUTimer();

        u64 CPUPaseJSONStart = ReadCPUTimer();
            int at = 0;
            auto json = parse(json_str, at, json_str.size()-1);
            auto& map = std::get<JSON::Map>(json);
            assert(map.find("pairs") != map.end());
            auto& pairs = std::get<JSON::Array>(map["pairs"]);
        u64 CPUParseJSONEnd = ReadCPUTimer();


        u64 CPUHaversineStart = ReadCPUTimer();
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
        u64 CPUHaversineEnd = ReadCPUTimer();

    u64 CPUTotalEnd = ReadCPUTimer();
    u64 OSTotalEnd = ReadOSTimer();

    std::cout << "Pair count: " << pairs.size() << "\n";
    std::cout << "Expected sum: " << average << "\n";

    f64 total_cpu = 1000.0*(f64)(CPUTotalEnd - CPUTotalStart)/(f64)CPUFreq;//in milliseconds
    f64 total_os = 1000.0*(f64)(OSTotalEnd - OSTotalStart)/(f64)OSFreq;
    printf("Total time: cpu:%.4fms/os:%.4fms (CPU freq %llu)\n", 
        total_cpu, 
        total_os, 
        CPUFreq);

    f64 read_file_cpu = 1000.0f * (f64)(CPUReadFileEnd - CPUReadFileStart)/(f64)CPUFreq;
    printf("File: %.4f (%.4f %%)\n", read_file_cpu, 100.0 * read_file_cpu/total_cpu);
    f64 parse_json_cpu = 1000.0f * (f64)(CPUParseJSONEnd - CPUPaseJSONStart)/(f64)CPUFreq;
    printf("parse: %.4f (%.4f %%)\n", parse_json_cpu, 100.0 * parse_json_cpu/total_cpu);
    f64 haversine_cpu = 1000.0f * (f64)(CPUHaversineEnd - CPUHaversineStart)/(f64)CPUFreq;
    printf("sum: %.4f (%.4f %%)\n", haversine_cpu, 100.0 * haversine_cpu/total_cpu);
    return 0;
}
