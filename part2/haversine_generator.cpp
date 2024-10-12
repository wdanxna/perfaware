#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <cstdlib>
#include <assert.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */
    
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}

struct haversine_pair
{
    f64 X0, Y0;
    f64 X1, Y1;
};

struct cluster {
    f64 x, y;//center
    f64 dx, dy;//range
    size_t size;//number of points
    std::vector<haversine_pair> pairs;
    std::vector<f64> haversine;
};

void dumpJSON(std::vector<cluster>& clusters) {
    std::string mode = clusters.size() == 1 ? "fix" : "flex";
    std::ostringstream ss;
    ss << "data_" << std::to_string(clusters.size()) << "_" << mode << ".json";
    std::ofstream out(ss.str());

    std::ostringstream content;
    if (out.is_open()) {  
        content << "{\"pairs\":[";
            for (auto& cluster : clusters) {
                for (auto& pair : cluster.pairs) {
                    content << "{\"X0\":" << pair.X0 << ", \"Y0\":" << pair.Y0 <<
                            ", \"X1\":" << pair.X1 << ", \"Y1\":" << pair.Y1 << "},";
                }
            }
        auto tmp = content.str();
        tmp.pop_back();
        out << tmp;
        out << "]}";
    }
    out.close();
}

cluster gen_cluster(
    f64 x, 
    f64 y,
    f64 dx, 
    f64 dy,
    size_t count,
    std::mt19937& generator) {
    cluster ret {x, y, dx, dy, count, std::vector<haversine_pair>(count)};


    //poplute points
    //calculate x, y distribution
    std::uniform_real_distribution<f64> distx(x-dx/2.0, x+dx/2.0);
    std::uniform_real_distribution<f64> disty(y-dy/2.0, y+dy/2.0);

    //generate 'count' number of point pairs into cluster
    for (int i = 0; i < count; i++) {
        ret.pairs[i] = {
            distx(generator), disty(generator),
            distx(generator), disty(generator)};

        auto& back = ret.pairs[i];
        ret.haversine.push_back(
            ReferenceHaversine(back.X0, back.Y0, back.X1, back.Y1, 6372.8)
        );
    }
    return ret;
}

int main (int argc, char* argv[]) {
    //[uniform/cluster] [random seed] [number of pairs to generate]
    assert(argc == 4);
    const std::string mode = argv[1];
    const auto seed = strtoul(argv[2], nullptr, 10);
    const auto count = strtoul(argv[3], nullptr, 10);

    std::mt19937 generator(seed);
    std::uniform_real_distribution<f64> dist(0.0, 360.0);


    std::vector<cluster> clusters;
    bool uniform_gen = (mode == "uniform");
    int cluster_count = uniform_gen ? 1 : 64;
    for (int i = 0; i < cluster_count; i++) {
        f64 x = uniform_gen ?  180.0 : dist(generator);
        f64 y = uniform_gen? 180.0 : dist(generator);
        f64 range = uniform_gen? 360.0 : 30.0;
        //Did not consider cluster overlapping
        clusters.push_back(gen_cluster(x, y, range, range, uniform_gen ? count : std::max(1ul, count/64), generator));
    }

    dumpJSON(clusters);

    //report the average haversine
    f64 sum = 0.0;
    f64 total = 0.0;
    for (int i = 0; i < clusters.size(); i++) {
        sum += std::accumulate(clusters[i].haversine.begin(), clusters[i].haversine.end(), 0.0);
        total += clusters[i].haversine.size();
    }
    auto average = sum / total;

    std::cout << "Method: " << mode << "\n";
    std::cout << "Random seed: " << seed << "\n";
    std::cout << "Pair count: " << count << "\n";
    std::cout << "Expected sum: " << average << "\n";
    return 0;
}