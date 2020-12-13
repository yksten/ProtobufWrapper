#include <iostream>
#include <assert.h>
#include "serialize.h"
#include "decoder.h"
#include "encoder.h"

#ifdef _MSC_VER
#include "benchmark.h"
#endif


enum EnumType {
    ET1,
    ET2,
    ET3,
};

struct struExample {
    uint32_t _id;
    std::string _str;
    float _i;
    double _num;

    template<typename T>
    void serialize(T& t) {
        t & SERIALIZATION(1, _id) & SERIALIZATION(2, _str) & SERIALIZATION(3, _i) & SERIALIZATION(4, _num);
    }
};

struct struExamples {
    std::vector<struExample> _v;
    std::map<int, struExample> _m;

    template<typename T>
    void serialize(T& t) {
        t & SERIALIZATION(1, _v) & SERIALIZATION(2, _m);
    }
};

void test() {
    struExample item;
    item._id = ET2;
    item._str = "example";
    item._i = 9.7f;
    item._num = 19.8f;
    struExamples items;
    items._v.push_back(item);
    items._m[1] = item;
    item._id = ET3;
    item._str = "afexample";
    item._i = 5.7f;
    item._num = 89.8f;
    items._v.push_back(item);
    items._m[2] = item;

    std::string buffer;
    serialize::PBEncoder encoder(buffer);
    encoder << items;

    struExamples items2;
    serialize::PBDecoder decoder(buffer);
    bool b = decoder >> items2;
    bool b2 = decoder >> items;
    assert(b);
}
#ifdef _MSC_VER
static void pb_encode(benchmark::State& st) {
    struExample item;
    item._id = ET2;
    item._str = "example";
    item._i = 9.7f;
    item._num = 19.8f;
    struExamples items;
    items._v.push_back(item);
    items._m[1] = item;
    item._id = ET3;
    item._str = "afexample";
    item._i = 5.7f;
    item._num = 89.8f;
    items._v.push_back(item);
    items._m[2] = item;
    while (st.KeepRunning()) {
        std::string buffer;
        serialize::PBEncoder encoder(buffer);
        bool b = encoder << items;
    }
}

static void pb_decode(benchmark::State& st) {
    struExample item;
    item._id = ET2;
    item._str = "example";
    item._i = 9.7f;
    item._num = 19.8f;
    struExamples items;
    items._v.push_back(item);
    items._m[1] = item;
    item._id = ET3;
    item._str = "afexample";
    item._i = 5.7f;
    item._num = 89.8f;
    items._v.push_back(item);
    items._m[2] = item;

    std::string buffer;
    serialize::PBEncoder encoder(buffer);
    bool b = encoder << items;
    while (st.KeepRunning()) {
        struExamples items2;
        serialize::PBDecoder decoder(buffer);
        bool b = decoder >> items2;
    }
}
BENCHMARK(pb_encode);
BENCHMARK(pb_decode);

#endif

int main(int argc, char** argv) {
#ifdef _MSC_VER
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();

    system("pause");
#else
    test();
#endif
    return 0;
}