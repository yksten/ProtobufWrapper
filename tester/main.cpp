#include <iostream>
#include <assert.h>
#include "serialize.h"
#include "decoder.h"
#include "encoder.h"

#ifdef _MSC_VER
#include <io.h>
#endif
#include <fstream>

#include "benchmark/benchmark.h"

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

//template<typename T>
//void serialize(T& t, struExample& item) {
//    t & SERIALIZATION(1, item._id) & SERIALIZATION(2, item._str) & SERIALIZATION(3, item._i) & SERIALIZATION(4, item._num);
//}

struct struExamples {
    std::vector<struExample> _v;
    std::map<int, struExample> _m;

    template<typename T>
    void serialize(T& t) {
        t & SERIALIZATION(1, _v) & SERIALIZATION(2, _m);
    }
};

//template<typename T>
//void serialize(T& t, struExamples& items) {
//    t & SERIALIZATION(1, items._v);
//    t & SERIALIZATION(2, items._m);
//}

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

//int main(int argc, char* argv[]) {
//    if (argc > 1)
//    {
//        std::ifstream file;
//        file.open(argv[1], std::ios::in | std::ios::binary);
//        assert(file.is_open());
//        file.seekg(0, file.end);
//        int length = file.tellg();
//        file.seekg(0, file.beg);
//        char* szBuffer = new char[length];
//        file.read(szBuffer, length);
//        file.close();
//
//        struExamples items;
//        serialization::PBDecoder decoder((uint8_t*)szBuffer, length);
//        decoder >> items;
//        delete[] szBuffer;
//    }
//
//    return 0;
//}

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
    encoder << items;
    while (st.KeepRunning()) {
        struExamples items2;
        serialize::PBDecoder decoder(buffer);
        bool b = decoder >> items2;
    }
}

BENCHMARK(pb_encode);
BENCHMARK(pb_decode);

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();

#ifdef _MSC_VER
    system("pause");
#endif
    return 0;
}