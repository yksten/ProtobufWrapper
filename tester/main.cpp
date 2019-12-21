#include <iostream>
#include <assert.h>
#include "serialize.h"
#include "decoder.h"
#include "encoder.h"

#include <io.h>
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
    t & SERIALIZETYPE(1, items._v);
}

int main(int argc, char* argv[]) {
    //if (argc > 1)
    //{
    //    std::ifstream file;
    //    file.open(argv[1], std::ios::in | std::ios::binary);
    //    assert(file.is_open());
    //    file.seekg(0, file.end);
    //    int length = file.tellg();
    //    file.seekg(0, file.beg);
    //    char* szBuffer = new char[length];
    //    file.read(szBuffer, length);
    //    file.close();

    //    struExamples items;
    //    serialization::PBDecoder decoder(szBuffer, length);
    //    decoder >> items;
    //    delete[] szBuffer;
    //}

    {
        struExample item;
        item._id = 1;
        item._str = "example";
        item._i = 2.5;
        item._num = 9.5;
        struExamples items;
        items._v.push_back(item);
        items._v.push_back(item);

        serialization::PBEncoder encoder;
        encoder << items;

        struExamples items2;
        serialization::PBDecoder decoder(encoder.data(), encoder.size());
        decoder >> items2;
    }

    return 0;
}