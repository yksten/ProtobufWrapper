#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__
#include <stdint.h>

namespace serialization {
    enum {
        WT_VARINT             = 0, //int32,int64,uint32,uint64,sint32,sin64,bool,enum
        WT_64BIT              = 1, //fixed64,sfixed64,double
        WT_LENGTH_DELIMITED   = 2, //string,bytes,embedded messages,packed repeated fields
        WT_START_GROUP        = 3, 
        WT_END_GROUP          = 4, 
        WT_32BIT              = 5, //fixed32,sfixed32,float
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

    template<typename VALUE>
    class serializePair {
        bool _pack;
        const uint32_t _tag;
        VALUE& _value;
    public:
        serializePair(uint32_t tag, VALUE& value) :_pack(false), _tag(tag), _value(value) {}
        serializePair(uint32_t tag, VALUE& value, bool pack) :_pack(pack), _tag(tag), _value(value) {}

        uint32_t tag() const { return _tag; }
        VALUE& value() { return _value; }
        const VALUE& value() const { return _value; }
    };

    template<typename VALUE>
    inline serializePair<VALUE> makePair(uint32_t tag, VALUE& value) {
        return serializePair<VALUE>(tag, value);
    }

    //template<typename VALUE>
    //inline serializePair<VALUE> makePair(uint32_t tag, VALUE& value, bool pack) {
    //    return serializePair<VALUE>(tag, value, pack);
    //}
}


#define SERIALIZETYPE(tag, value) serialization::makePair(tag, value)


#endif