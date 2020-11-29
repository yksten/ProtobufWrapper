#ifndef __TEST_TEST1__H_
#define __TEST_TEST1__H_

#include<stdint.h>
#include <string>
#include <vector>
#include <map>

namespace testStruct {

    struct struExample {
        struExample(): id(), f(), db() {}
        int32_t id;
        std::string str;
        float f;
        double db;

        template<typename T>
        void serialize(T& t) {
            t & SERIALIZATION(1, id) & SERIALIZATION(2, str) & SERIALIZATION(3, f) & SERIALIZATION(4, db);
        }
    };

    struct struExamples {
        struExamples() {}
        std::vector<struExample> v;
        std::map<int32_t, struExample> m;

        template<typename T>
        void serialize(T& t) {
            t & SERIALIZATION(1, v) & SERIALIZATION(2, m);
        }
    };

}

#endif
