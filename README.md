My homework assignment for performance awareness programming
## 20/10/2024
### Repetition test
setup the repetition test framework, test on 3 read file functions(`fread`, `CFReadStream` and `read`) to see their bandwidth. The results is as following:

```
CPU Freq: 4012858420, FileSize: 5358880
--- ReadFile ---
Max: 2581026 (0.643189ms) 7.759534GB/s            
Min: 2581026 (0.643189ms) 7.759534GB/s            
Min: 803926 (0.200337ms) 24.912191GB/s            
Min: 552354 (0.137646ms) 36.258555GB/s            
Min: 543390 (0.135412ms) 36.856692GB/s          
Min: 456210 (0.113687ms) 43.899866GB/s            

Min: 456210 (0.113687ms) 43.899866GB/s
Max: 2581026 (0.643189ms) 7.759534GB/s
Avg: 480861 (0.119830ms) 41.649389GB/s

--- fread ---
Max: 527215 (0.131381ms) 37.987459GB/s            
Min: 527215 (0.131381ms) 37.987459GB/s            
Max: 578704 (0.144212ms) 34.607603GB/s            
Min: 524258 (0.130645ms) 38.201721GB/s            
Min: 502697 (0.125272ms) 39.840218GB/s            
Min: 499248 (0.124412ms) 40.115450GB/s            
Min: 496688 (0.123774ms) 40.322210GB/s            
Min: 487423 (0.121465ms) 41.088660GB/s            
Min: 473583 (0.118016ms) 42.289436GB/s            
Min: 471485 (0.117494ms) 42.477614GB/s            
Min: 468871 (0.116842ms) 42.714431GB/s            
Max: 606575 (0.151158ms) 33.017447GB/s            
Max: 620414 (0.154607ms) 32.280958GB/s            
Max: 991510 (0.247083ms) 20.199048GB/s            
Max: 1133022 (0.282348ms) 17.676231GB/s            
Max: 1450576 (0.361482ms) 13.806624GB/s            
Max: 1903586 (0.474372ms) 10.520963GB/s            
Min: 461224 (0.114937ms) 43.422628GB/s            
Min: 461164 (0.114922ms) 43.428277GB/s            
Min: 460837 (0.114840ms) 43.459093GB/s            
Min: 460622 (0.114787ms) 43.479378GB/s            
Min: 460480 (0.114751ms) 43.492786GB/s            
Min: 460166 (0.114673ms) 43.522464GB/s            
Min: 459389 (0.114479ms) 43.596077GB/s            
Min: 458917 (0.114362ms) 43.640916GB/s            
Min: 458014 (0.114137ms) 43.726956GB/s            
Min: 457806 (0.114085ms) 43.746823GB/s            
Min: 457582 (0.114029ms) 43.768238GB/s            
Min: 457198 (0.113933ms) 43.804999GB/s            
Min: 457048 (0.113896ms) 43.819376GB/s            
Max: 1996930 (0.497633ms) 10.029174GB/s            
Min: 456464 (0.113750ms) 43.875438GB/s            
Min: 455900 (0.113610ms) 43.929717GB/s            
Min: 455335 (0.113469ms) 43.984227GB/s            
Min: 454798 (0.113335ms) 44.036161GB/s            
Max: 2240814 (0.558408ms) 8.937626GB/s            

Min: 454798 (0.113335ms) 44.036161GB/s
Max: 2240814 (0.558408ms) 8.937626GB/s
Avg: 475358 (0.118459ms) 42.131515GB/s

--- read ---
Max: 491694 (0.122530ms) 40.731752GB/s            
Min: 491694 (0.122530ms) 40.731752GB/s            
Min: 478903 (0.119342ms) 41.819655GB/s            
Min: 471348 (0.117459ms) 42.489961GB/s            
Max: 512545 (0.127726ms) 39.074731GB/s            
Max: 524453 (0.130693ms) 38.187517GB/s            
Max: 541325 (0.134898ms) 36.997290GB/s            
Min: 466698 (0.116301ms) 42.913315GB/s            
Min: 466270 (0.116194ms) 42.952706GB/s            
Min: 461590 (0.115028ms) 43.388197GB/s            
Max: 576885 (0.143759ms) 34.716725GB/s            
Max: 606050 (0.151027ms) 33.046049GB/s            
Max: 746412 (0.186005ms) 26.831774GB/s            
Max: 748996 (0.186649ms) 26.739206GB/s            
Max: 969945 (0.241709ms) 20.648138GB/s            
Max: 988144 (0.246244ms) 20.267854GB/s            
Max: 1099593 (0.274017ms) 18.213610GB/s            
Max: 1316212 (0.327999ms) 15.216058GB/s            
Min: 461002 (0.114881ms) 43.443538GB/s            
Min: 460657 (0.114795ms) 43.476075GB/s            
Min: 459605 (0.114533ms) 43.575588GB/s            
Min: 458638 (0.114292ms) 43.667463GB/s            
Min: 456804 (0.113835ms) 43.842782GB/s            
Min: 456067 (0.113651ms) 43.913631GB/s            
Min: 455558 (0.113525ms) 43.962696GB/s            
Min: 454434 (0.113244ms) 44.071434GB/s            
Min: 453934 (0.113120ms) 44.119978GB/s            
Min: 453202 (0.112937ms) 44.191239GB/s            
Max: 1404584 (0.350021ms) 14.258712GB/s            
Max: 1516538 (0.377920ms) 13.206104GB/s            
Max: 1668470 (0.415781ms) 12.003547GB/s            
Max: 2113679 (0.526727ms) 9.475213GB/s            
Max: 2431256 (0.605866ms) 8.237536GB/s            
Min: 452451 (0.112750ms) 44.264590GB/s            
Min: 451934 (0.112621ms) 44.315228GB/s            

Min: 451934 (0.112621ms) 44.315228GB/s
Max: 2431256 (0.605866ms) 8.237536GB/s
Avg: 468726 (0.116806ms) 42.727628GB/s
```

Observations: 
- Whoever executed first has the slowest bandwidth recorded (~ 7GB/s)
- Subsequent reads (no matter which function as long as it's not the first) can reach around 44GB/s peak
- After few reads, the bandwidth will regress to ~ 7GB/s before jumping back to 44GB/s

## 19/10/2024
### Calculate data throughput
Add data throughput support for profiler

`throughput = data_size / elapsed_time`

By adding another parameter to `TimeBlock` that specify data size, store `bytes` in the anchor object. when calculate the throughput at report stage, get inclusive elapsed time in seconds by `seconds = inclusive_elpased / cpu_freq`, calcuate throughput in GB/s by `throughput = (bytes/Gigabytes)/seconds`.

## 18/10/2024

### recursive profiler
current profiler cannot deal with recursion:
![alt text](part2/recursion.jpeg "recursion")
we can't simply calculate the `cost of foo without children` by `a-b` since it doesn't account for the `d`, hence will not reflect the real `cost of foo without children`. Instead we should do `a - b + d` or `a - c`.

setup a counter `depth` to keep track of how many times current function has been entered before it returns, if depth > 0, it means the function is called recursively.

setup a `real_elapsed` variable to track the elapsed time only when the top-level function returns (when depth == 0), ignoring any recursive cost of itself, since the top-level cost will cover them all.

use the `elapsed` variable to accumulate all elapsed time as before.
`elapsed = a + d`

use the `children_elapsed` variable to accumulate all elapsed time of children as before.
`children_elapsed = b`

calculate the cost of foo without children: `elapsed - children_elapsed = a + d - b = a - c`

use the `real_elpased` to represent the total elapsed of foo (instead of `elapsed` as before)

### Improvement 
Instead of using a nested counter `depth` to tell if it's the outmost function, we use a `elapsed_inclusive` counter to represent the outmost function's cost. Whenever a `scoped_timer` opens, its value is read and stored (in `scoped_timer` object), and when the block ends, put the stored value **plus** the elapsed time and write it back to the anchor, thus the outmost function will always overwrite whatever value its recursive self has written and we save a write operation everytime we open a scope timer.

Since we use new variable `elapsed_inclusive` to track the total elapsed time a function cost, the only purpose of variable `elpased` and `children_elapsed` is to calculate **exclusive** cost of the function(because under the recursion, `elapsed` won't represent true cost of a function). We could collapse these 2 variables into 1 variable `elapsed_exclusive`, which accumulated on its own anchor but substract from its parent's whenever the block ends.

### The interpretation of the report
```
Total elapsed: 2739.46ms
    parse_array[1]: 350.0628ms (12.7785% exclusive, 81.9342% inclusive)
    parse_dict[100033]: 1187.8129ms (43.3594% exclusive, 96.0583% inclusive)
    parse_number[400128]: 302.5226ms (11.0432% exclusive, 11.0432% inclusive)
    parse[500162]: 791.0906ms (28.8776% exclusive, 96.0587% inclusive)
    read_file[1]: 1.6470ms (0.0601% exclusive, 0.0601% inclusive)
    parse_haversine_pairs[1]: 46.5730ms (1.7001% exclusive, 99.9390% inclusive)
    lookup and convert[1]: 59.7245ms (2.1802% exclusive, 2.1802% inclusive)
```
It reads as `<function name>[<hit count>]: <exclusive time in ms> (<exclusive ratio> exclusive, <inclusive ratio> inclusive)`

### Add kill switch
Add a kill switch to easily turn on and off the profiler, this allows us to do AB test to see how much overhead the profiler markups incurred. Find a sweetspot between granularity of profiling information and overhead.

Basically, do not put markups on those recursive function that get million calls.


## 17/10/2024
### Modify the profiler
I modified the profiler to align with Casy's implementation:

1. use array instead of std::vector
2. implement nested profiler use a global parent pointer

## 15/10/2024
### Basic instrument utilities 14:48
implement a utility that facilitate instrumentation-based profiling.

based on `ReadCPUTimer` implemented previously, wirte a little RAII class which record between its construction and destruction, inside destructor, add the elpased value into a global variable.

write a macro to quick profile a function


### Understand mach_absolute_time 11:00
As the requirement of homework assignment, I need to investigate how performance counter works under Macos.

The equivalence of `QueryPerformanceCounter` in windows on macos is `mach_absolute_time()`.

First, I setup the routine to esitmate the counter's frequency as previously did. It shows that the frequency
is about 24MHz. Then I prob into the `mach_absolute_time()` use debugger to see where does the value come from.

The assembly sequence of `mach_absolute_time` on my m3 macbook is as follow:
```
movk x3, #0x0, lsl #48
movk x3, #0xf, lsl #32
movk x3 #0xffff, lsl #16
movk x3, #0xc088
ldrb w2, [x3, #0x8]
...
cmp x2, #0x3
b.eq 0x18062e2c0;
...
//At address 2c0
ldr x1, [x3]
mrs x0, S3_4_C15_C10_6
...
add x0, x0, x1
ret
```
The first several `movk`s construct a 64bit value as `0xfffffc088`. on darwin, the `0xfffffc000` is 'common page' that the darwin kernel maps it into userspace: https://github.com/apple/darwin-xnu/blob/2ff845c2e033bd0ff64b5b6aa6063a1f8f65aa32/osfmk/arm/cpu_capabilities.h#L203

so the `0xfffffc088` is _COMM_PAGE_TIMEBASE_OFFSET (https://github.com/apple/darwin-xnu/blob/2ff845c2e033bd0ff64b5b6aa6063a1f8f65aa32/osfmk/arm/cpu_capabilities.h#L230)

the `0xfffffc088 + 0x8` is _COMM_PAGE_USER_TIMEBASE (https://github.com/apple/darwin-xnu/blob/2ff845c2e033bd0ff64b5b6aa6063a1f8f65aa32/osfmk/arm/cpu_capabilities.h#L231)

the routine then jump based on the `user_timebase`, read value from system register `S3_4_C15_C10_6`, then add an offset on it before return it as the value.

seems like no special calculation happens here, just pure register read.

## 14/10/2024
Import a cpu cycle counter from https://github.com/lemire/Code-used-on-Daniel-Lemire-s-blog/blob/master/2023/03/21/performancecounters/apple_arm_events.h 

which need to be run with root privilege

write a function to estimate cpu frequency by:
1. use the OS timer as the reference point, we know that OS timer's frequency is 1000000/second (1 nanosecond resolution)
2. write a while loop in a short time frame (e.g. 100ms) to get how many ticks the OS timer has gone by (os_elapsed), as well as how many ticks the CPU timer has gone by (cpu_elpased)
3. get ratio by `cpu_elapsed / os_elapsed`, this tells us how many ticks has cpu gone per OS tick. Then multiplied that with OS frequency `os_freq * cpu_elapsed/os_elapsed` yields estimated cpu frequency per second

Use the estimated cpu frequency to further estimate the haversine processor:
1. read cpu time before and after an operation (e.g. json parsing) and get the difference.
2. calculate the elasped time in millisecond as  `1000.0 * diff/cpu_freq`

## 13/10/2024
write a simple json parser `json_parser.cpp`, supporting null, number, string, array, map.

`haversine_compare.cpp` use the parser to parse the generated json file and compute its reference haversine average(expect sum)

## 12/10/2024
write a haversine json generator, implemented 2 generating mode: unifrom and cluster, in which, uniform mode generate
haversine piars uniformally across entire x and y, whereas cluster mode generate first by choosing 64 cluster randomly
then populate each cluster with pairs that confined inside a specific range (30 degree range both vertically and horizontally).
it then will calculate the haversine value using reference haversine function provided for each pair during generation. Lastly, it will calculate a average among all haversine value and output a report to standard output.


## 11/10/2024
calcuate estimate cycle count, completing LISTING 56 and challenge LISTING 57

## 10/10/2024

simulating add, sub, cmp. which all of three share the almost same code path.
again, runtime dispatch seems neccessary to distinguish between different register sizes

---

Redesigned register storage, make it more explicit.
implement ip manipulation, simulating jne instruction, completing LISTING 48, 49.

---

Unify the operand access throuh rm_access object for `MOV`. completing LISTING 51, 52.
simulating memory access without considering segment registers and 8bits registers, so only 64K is addressable.

---

Successfully running LISTING 54 a "real" program, creating a 64x64x4 image, dumping the raw image into a file and check with GIMP

![alt text](part1/dump.jpg "Title")

## 9/10/2024
I decided to use Casys's Sim86 as basis for future assignments since its data structure is better designed and I want to study its code and design style. I imported its Sim86 as library into my homework workspace, write a main.cpp for assignment 4 which is simulating `MOV reg, imm` and `MOV reg, reg` instructions.

Implementation: use Sim86 to decode (deseriallize) the binary into data structure one instruction at a time, setup a context data structure to represent a 8086 machine, which now is just a 1MB buffer for memory and 8*2 bytes buffer for registers. write an `execute` function which receives a instruction and a context, which will update the context based on the instruction.

the current `execute` is implemented with just 2 ugly if-elses for simulating the 2 MOVs that LISTING 43 required.

----
Do challenge LISTING 45, be able to read/write into 8bit registers. write a utility tempalte function `read_register` and `wirte_register` that can operate on buffers in accordance with type been specified. A dynamic dispatch (with if-elses) seems needed in order to discern which register to access

## 06/10/2024
Complete assignment 1, 2 and challenge assignment:
An 8086 disassembler, can decode all MOV instructions now

assignment 3: implement ADD, SUB, CMP, JNZ decoding



