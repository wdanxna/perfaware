struct Profiler {
    struct Anchor {
        const char* name;
        u64 hit_count;
        u64 elapsed_inclusive;//containing all children's cost
        u64 elapsed_exclusive;//excluded all children's cost
        u64 bytes;//how many bytes does this block process
        u64 page_fault;
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
                    if (sec.page_fault) {
                        printf(", PF: %llu", sec.page_fault);
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
    u64 page_fault_cnt;
    scope_timer(const char* name, u64 bytes, u64 index) {
        this->name = name;
        this->id = index;
        this->bytes = bytes;
        begin = ReadCPUTimer();
        parent = globalParent;
        globalParent = index;
        page_fault_cnt = ReadOSPageFaultCount();

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
        u64 delta_pf = ReadOSPageFaultCount() - page_fault_cnt;
        sec.page_fault += delta_pf;

        //accumulate to parent
        if (parent != 0) {
            //0 is reserved
            auto& p = profiler.sections[parent];
            p.elapsed_exclusive -= elapsed;
            p.page_fault -= delta_pf;
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