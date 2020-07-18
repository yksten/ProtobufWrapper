#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__
#include <stdint.h>
#include <vector>

namespace serialization {
    enum {
        WT_VARINT             = 0,    // int32,int64,uint32,uint64,sint32,sin64,bool,enum
        WT_64BIT              = 1,    // fixed64,sfixed64,double
        WT_LENGTH_DELIMITED   = 2,    // string,bytes,embedded messages,packed repeated fields
        WT_START_GROUP        = 3,    // Groups(deprecated)
        WT_END_GROUP          = 4,    // Groups(deprecated)
        WT_32BIT              = 5,    // fixed32,sfixed32,float
    };

    enum {
        TYPE_VARINT  = 0,    // int32,int64,uint32,uint64,bool,enum
        TYPE_SVARINT = 1,    // sint32,sin64
        TYPE_FIXED32 = 2,    // fixed32,sfixed32
        TYPE_FIXED64 = 3,    // fixed64,sfixed64
        TYPE_BYTES   = 4,    // bytes
        TYPE_PACK    = 5,    // repaeted [pack=true]
    };

    template <typename T>
    class isMessage {
    private:
        template < typename C, C&(C::*)(const C&) = &C::operator=> static char check(C*);
        template<typename C> static int32_t check(...);
    public:
        enum { YES = (sizeof(check<T>(static_cast<T*>(0))) == sizeof(char)) };
        enum { WRITE_TYPE = (YES == 0) ? WT_VARINT : WT_LENGTH_DELIMITED };
    };

    template<>
    class isMessage<std::string> {
    public:
        enum { YES = 0, WRITE_TYPE = WT_LENGTH_DELIMITED };
    };

    template<>
    class isMessage<float> {
    public:
        enum { YES = 0, WRITE_TYPE = WT_32BIT };
    };

    template<>
    class isMessage<double> {
    public:
        enum { YES = 0, WRITE_TYPE = WT_64BIT };
    };

    struct access {
        template<class T, class C>
        static void serialize(T& t, C& c) {
            c.serialize(t);
        }
    };

    template<class T, class C>
    inline void serialize(T& t, C& c) {
        access::serialize(t, c);
    }

    template<class T, class C>
    inline void serializeWrapper(T& t, C& c) {
        serialize(t, c);
    }

    template<typename T>
    struct TypeTraits {
        typedef T Type;
    };

    template<typename T>
    struct TypeTraits<std::vector<T> > {
        typedef std::vector<T> Type;
    };

    template<typename VALUE>
    class serializePair {
        const uint32_t _num;
        VALUE& _value;
        const uint32_t _type;
    public:
        serializePair(uint32_t num, VALUE& value) :_type(TYPE_VARINT), _num(num), _value(value) {}
        serializePair(uint32_t num, VALUE& value, uint32_t type) :_type(type), _num(num), _value(value) {}

        uint32_t type() const { return _type; }
        uint32_t num() const { return _num; }
        VALUE& value() { return _value; }
        const VALUE& value() const { return _value; }
    };

    template<typename VALUE>
    inline serializePair<VALUE> makePair(uint32_t num, VALUE& value) {
        return serializePair<VALUE>(num, value);
    }

    template<typename VALUE>
    inline serializePair<VALUE> makePair(uint32_t num, VALUE& value, int32_t type) {
        return serializePair<VALUE>(num, value, type);
    }
}


#define SERIALIZETYPE_2(num, value)         serialization::makePair(num, value)
#define SERIALIZETYPE_3(num, value, type)   serialization::makePair(num, value, type)


#define EXPAND(args) args
#define MAKE_TAG_COUNT(TAG, _3,_2,_1,N,...) TAG##N
#define SERIALIZETYPE(...) EXPAND(MAKE_TAG_COUNT(SERIALIZETYPE, __VA_ARGS__, _3,_2,_1) (__VA_ARGS__))

#endif