#include <iostream>
#include <assert.h>
#include "pb/test1.pb.h"

#include "encoder.h"
#include "decoder.h"
#include "serialize.h"
#include "pb/test1.struct.h"


int main(int argc, char* argv[]) {
    testStruct::struExample item;
    item.id = 1;
    item.str = "example";
    item.f = 9.7f;
    item.db = 19.8f;
    testStruct::struExamples items;
    items.v.push_back(item);
    items.m[1] = item;
    item.id = 2;
    item.str = "afexample";
    item.f = 5.7f;
    item.db = 89.8f;
    items.v.push_back(item);
    items.m[2] = item;

    serialization::BufferWrapper buffer;
    serialization::PBEncoder encoder(buffer);
    encoder << items;

    test::struExamples pbItems;
    bool ret = pbItems.ParseFromArray(buffer.data(), buffer.size());
    assert(ret);
    assert(items.v.size() == pbItems.v_size());
    for (uint32_t idx = 0; idx < pbItems.v_size(); ++idx) {
        const test::struExample& item = pbItems.v(idx);
        const testStruct::struExample& item2 = items.v.at(idx);
        assert(item.id() == item2.id);
        assert(item.str() == item2.str);
        assert(item.f() == item2.f);
        assert(item.db() == item2.db);
    }
    assert(items.m.size() == pbItems.m_size());
    const ::google::protobuf::Map< ::google::protobuf::int32, ::test::struExample >&pbMap = pbItems.m();
    for (auto it = pbMap.begin(); it != pbMap.end(); ++it) {
        const test::struExample& item = it->second;
        const testStruct::struExample& item2 = items.m[it->first];
        assert(item.id() == item2.id);
        assert(item.str() == item2.str);
        assert(item.f() == item2.f);
        assert(item.db() == item2.db);
    }
    std::string strBuffer;
    pbItems.SerializeToString(&strBuffer);

    testStruct::struExamples items2;
    serialization::PBDecoder decoder((const uint8_t*)strBuffer.data(), strBuffer.size());
    decoder >> items2;
    
    return 0;
}