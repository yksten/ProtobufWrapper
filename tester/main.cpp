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
    uint32_t id;
    std::string str;
    float f;
    double db;

    uint64_t getByteSize() const {
        uint64_t total_size = 0;
        if (id) {
            total_size += 1 + serialize::ByteSize(static_cast<uint64_t>(id));
        }
        if (!str.empty()) {
            total_size += 1 + serialize::ByteSize(str);
        }
        if (f) {
            total_size += 1 + 4;
        }
        if (db) {
            total_size += 1 + 8;
        }
        return total_size;
    }

    template<typename T>
    void serialize(T& t) {
        t & SERIALIZATION(1, id) & SERIALIZATION(2, str) & SERIALIZATION(3, f) & SERIALIZATION(4, db);
    }
};

struct struExamples {
    std::vector<struExample> v;
    std::map<int, struExample> m;

    uint64_t getByteSize() const {
        uint64_t total_size = 0;
        if (!v.empty()) {
            uint32_t size = (uint32_t)v.size();
            for (uint32_t i = 0; i < size; ++i) {
                total_size += 1 + serialize::ByteSize(v[i]);
            }
        }
        if (!m.empty()) {
            for (std::map<int, struExample>::const_iterator it = m.begin(); it != m.end(); ++it) {
                uint64_t temp = 1 + serialize::ByteSize(static_cast<uint64_t>(it->first)) + 1 + serialize::ByteSize(it->second);
                total_size += 1 + serialize::ByteSize(static_cast<uint64_t>(temp)) + temp;
            }
        }
        return total_size;
    }

    template<typename T>
    void serialize(T& t) {
        t & SERIALIZATION(1, v) & SERIALIZATION(2, m);
    }
};

void test() {
    struExample item;
    item.id = ET2;
    item.str = "example";
    item.f = 9.7f;
    item.db = 19.8f;
    struExamples items;
    items.v.push_back(item);
    items.m[1] = item;
    item.id = ET3;
    item.str = "afexample";
    item.f = 5.7f;
    item.db = 89.8f;
    items.v.push_back(item);
    items.m[2] = item;

    std::string buffer;
    serialize::PBEncoder encoder(buffer);
    encoder << items;

    struExamples items2;
    serialize::PBDecoder decoder(buffer);
    bool b = decoder >> items2;
    assert(b);
    bool b2 = decoder >> items;
    assert(b2);
}
#ifdef _MSC_VER
static void pb_encode(benchmark::State& st) {
    struExample item;
    item.id = ET2;
    item.str = "example";
    item.f = 9.7f;
    item.db = 19.8f;
    struExamples items;
    items.v.push_back(item);
    items.m[1] = item;
    item.id = ET3;
    item.str = "afexample";
    item.f = 5.7f;
    item.db = 89.8f;
    items.v.push_back(item);
    items.m[2] = item;

    while (st.KeepRunning()) {
        std::string buffer;
        serialize::PBEncoder encoder(buffer);
        bool b = encoder << items;
        assert(b);
    }
}

static void pb_decode(benchmark::State& st) {
    struExample item;
    item.id = ET2;
    item.str = "example";
    item.f = 9.7f;
    item.db = 19.8f;
    struExamples items;
    items.v.push_back(item);
    items.m[1] = item;
    item.id = ET3;
    item.str = "afexample";
    item.f = 5.7f;
    item.db = 89.8f;
    items.v.push_back(item);
    items.m[2] = item;

    std::string buffer;
    serialize::PBEncoder encoder(buffer);
    bool b = encoder << items;
    assert(b);
    while (st.KeepRunning()) {
        struExamples items2;
        serialize::PBDecoder decoder(buffer);
        bool b = decoder >> items2;
        assert(b);
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