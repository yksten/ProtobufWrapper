#include <iostream>
#include <assert.h>
#include "serialize.h"
#include "decoder.h"
#include "encoder.h"

#include "parse/proto.h"

#ifdef _MSC_VER
#include <io.h>
#endif
#include <fstream>


struct struExample {
    uint32_t _id;
    std::string _str;
    float _i;
    double _num;

    template<typename T>
    void serialize(T& t) {
        t & SERIALIZETYPE(1, _id) & SERIALIZETYPE(2, _str) & SERIALIZETYPE(3, _i) & SERIALIZETYPE(4, _num);
    }
};

template<typename T>
void serialize(T& t, struExample& item) {
    t & SERIALIZETYPE(1, item._id) & SERIALIZETYPE(2, item._str) & SERIALIZETYPE(3, item._i) & SERIALIZETYPE(4, item._num);
}

struct struExamples {
    std::vector<struExample> _v;
};

template<typename T>
void serialize(T& t, struExamples& items) {
    t & SERIALIZETYPE(1, items._v, serialization::TYPE_PACK);
}

int main(int argc, char* argv[]) {
    if (argc > 1)
    {
        std::ifstream file;
        file.open(argv[1], std::ios::in | std::ios::binary);
        assert(file.is_open());
        file.seekg(0, file.end);
        int length = file.tellg();
        file.seekg(0, file.beg);
        char* szBuffer = new char[length];
        file.read(szBuffer, length);
        file.close();

        struExamples items;
        serialization::PBDecoder decoder((uint8_t*)szBuffer, length);
        decoder >> items;
        delete[] szBuffer;
    }

    {
        struExample item;
        item._id = 1;
        item._str = "example";
        item._i = 9.7f;
        item._num = 19.8f;
        struExamples items;
        items._v.push_back(item);
        item._id = 2;
        item._str = "afexample";
        item._i = 5.7f;
        item._num = 89.8f;
        items._v.push_back(item);

        serialization::BufferWrapper buffer;
        serialization::PBEncoder encoder(buffer);
        encoder << items;

        proto::Message msg;
        bool b = msg.ParseFromBytes(buffer.data(), buffer.size());
        std::vector<proto::Message*>msgs = msg.GetMessageArray(1);
        for (uint16_t i = 0; i < msgs.size(); ++i) {
            proto::Message* item = msgs.at(i);
            uint32_t id = item->GetVarInt(1);
            std::string str = item->GetString(2);
            float f = item->GetFloat(3);
            double db = item->GetDouble(4);
        }

        struExamples items2;
        serialization::PBDecoder decoder(buffer.data(), buffer.size());
        decoder >> items2;
    }

    return 0;
}