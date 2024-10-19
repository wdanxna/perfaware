My homework assignment for performance awareness programming
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



