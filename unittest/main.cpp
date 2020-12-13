#include <iostream>
#include <assert.h>
#include "pb/test1.pb.h"

#include "encoder.h"
#include "decoder.h"
#include "serialize.h"
#include "pb/test1.struct.h"

//int main(int argc, char* argv[]) {
//    testStruct::struExample item;
//    item.id = 1;
//    item.str = "example";
//    item.f = 9.7f;
//    item.db = 19.8f;
//    testStruct::struExamples items;
//    items.v.push_back(item);
//    items.m[1] = item;
//    item.id = 2;
//    item.str = "afexample";
//    item.f = 5.7f;
//    item.db = 89.8f;
//    items.v.push_back(item);
//    items.m[2] = item;
//
//    std::string buffer;
//    serialize::PBEncoder encoder(buffer);
//    encoder << items;
//
//    testStruct::struExamples items2;
//    serialize::PBDecoder decoder(buffer);
//    decoder >> items2;
//
//    test::struExamples pbItems;
//    bool ret = pbItems.ParseFromArray(buffer.data(), buffer.size());
//    assert(ret);
//    assert(items.v.size() == pbItems.v_size());
//    for (uint32_t idx = 0; idx < pbItems.v_size(); ++idx) {
//        const test::struExample& item = pbItems.v(idx);
//        const testStruct::struExample& item2 = items.v.at(idx);
//        assert(item.id() == item2.id);
//        assert(item.str() == item2.str);
//        assert(item.f() == item2.f);
//        assert(item.db() == item2.db);
//    }
//    assert(items.m.size() == pbItems.m_size());
//    const ::google::protobuf::Map< ::google::protobuf::int32, ::test::struExample >&pbMap = pbItems.m();
//    for (auto it = pbMap.begin(); it != pbMap.end(); ++it) {
//        const test::struExample& item = it->second;
//        const testStruct::struExample& item2 = items.m[it->first];
//        assert(item.id() == item2.id);
//        assert(item.str() == item2.str);
//        assert(item.f() == item2.f);
//        assert(item.db() == item2.db);
//    }
//    std::string strBuffer;
//    pbItems.SerializeToString(&strBuffer);
//    
//    return 0;
//}



#include "benchmark.h"
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

    std::string buffer;
    serialize::PBEncoder encoder(buffer);
    bool ret = encoder << items;

    test::struExamples pbItems;
    ret = pbItems.ParseFromString(buffer);

    while (st.KeepRunning()) {
        std::string strBuffer;
        bool b = pbItems.SerializeToString(&strBuffer);
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
        test::struExamples pbItems;
        bool ret = pbItems.ParseFromString(buffer);
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