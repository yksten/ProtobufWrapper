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
        t & SERIALIZE(1, _id) & SERIALIZE(2, _str) & SERIALIZE(3, _i) & SERIALIZE(4, _num);
    }
};

template<typename T>
void serialize(T& t, struExample& item) {
    t & SERIALIZE(1, item._id) & SERIALIZE(2, item._str) & SERIALIZE(3, item._i) & SERIALIZE(4, item._num);
}

struct struExamples {
    std::vector<struExample> _v;
};

template<typename T>
void serialize(T& t, struExamples& items) {
    t & SERIALIZE(1, items._v, serialization::TYPE_PACK);
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

        struExamples items2;
        serialization::PBDecoder decoder(buffer.data(), buffer.size());
        decoder >> items2;
    }

    return 0;
}