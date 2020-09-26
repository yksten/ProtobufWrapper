#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__

#ifdef _MSC_VER
#ifdef EXPORTAPI 
#define EXPORTAPI _declspec(dllimport)
#else 
#define EXPORTAPI _declspec(dllexport)
#endif
#else
#define EXPORTAPI
#endif

#include <stdint.h>
#include <vector>

#define SERIALIZE_2(num, value)              serialization::makeItem(#value, num, value)
#define SERIALIZE_3(num, value, typeOrHas)   serialization::makeItem(#value, num, value, typeOrHas)
#define SERIALIZE_4(num, value, type, has)   serialization::makeItem(#value, num, value, type, has)

#define EXPAND(args) args
#define MAKE_TAG_COUNT(TAG, _4, _3,_2,_1,N,...) TAG##N
#define SERIALIZE(...) EXPAND(MAKE_TAG_COUNT(SERIALIZE, __VA_ARGS__, _4, _3,_2,_1) (__VA_ARGS__))

namespace serialization {

    enum {
        TYPE_VARINT  = 0,    // int32,int64,uint32,uint64,bool,enum
        TYPE_SVARINT = 1,    // sint32,sin64
        TYPE_FIXED32 = 2,    // fixed32,sfixed32
        TYPE_FIXED64 = 3,    // fixed64,sfixed64
        TYPE_BYTES   = 4,    // bytes
        TYPE_PACK    = 5,    // repaeted [pack=true]
    };

    class EXPORTAPI BufferWrapper {
        std::vector<uint8_t> _buffer;
        size_t _index;
        enum { INITIALSIZE = 8 };

        bool _bCalculateFlag;
        size_t _cumtomFieldSize;

    public:
        explicit BufferWrapper(size_t nSize = INITIALSIZE);

        uint8_t* data() { return &(*_buffer.begin()); }
        const uint8_t* data() const { return &(*_buffer.begin()); }
        size_t size() const { return _index; }

        void append(const void* data, size_t len);

        void startCalculateSize();
        std::pair<bool, size_t> getCustomField() const { return std::pair<bool, size_t>(_bCalculateFlag, _cumtomFieldSize); }
        void setCustomField(const std::pair<bool, size_t>& pair) { _bCalculateFlag = pair.first; _cumtomFieldSize = pair.second; }
    };

    template<typename VALUE>
    struct serializeItem {
        serializeItem(const char* sz, uint32_t n, VALUE& v, bool* b) : name(sz), num(n), value(v), type(TYPE_VARINT), bHas(b) {}
        serializeItem(const char* sz, uint32_t n, VALUE& v, uint32_t t, bool* b) : name(sz), num(n), value(v), type(t), bHas(b) {}

        const char* name;
        const uint32_t num;
        VALUE& value;
        const uint32_t type;
        bool* bHas;

        void setHas(bool b) {   //proto2 has
            if (bHas)
                *bHas = b;
        }
        void setValue(const VALUE& v) {
            value = v;
            setHas(true);
        }
    };

    template<typename VALUE>
    inline serializeItem<VALUE> makeItem(const char* sz, uint32_t num, VALUE& value, bool* b = NULL) {
        return serializeItem<VALUE>(sz, num, value, b);
    }

    template<typename VALUE>
    inline serializeItem<VALUE> makeItem(const char* sz, uint32_t num, VALUE& value, int32_t type, bool* b = NULL) {
        return serializeItem<VALUE>(sz, num, value, type, b);
    }

    namespace internal {

        // These are defined in:
        // https://developers.google.com/protocol-buffers/docs/encoding
        enum WireType {
            WIRETYPE_VARINT = 0,                // int32,int64,uint32,uint64,sint32,sin64,bool,enum
            WIRETYPE_64BIT = 1,                 // fixed64,sfixed64,double
            WIRETYPE_LENGTH_DELIMITED = 2,      // string,bytes,embedded messages,packed repeated fields
            WIRETYPE_GROUP_START = 3,           // Groups(deprecated)
            WIRETYPE_GROUP_END = 4,             // Groups(deprecated)
            WIRETYPE_32BIT = 5,                 // fixed32,sfixed32,float
        };

        template<typename From, typename To>
        class is_convertible {
            typedef char one;
            typedef int  two;

            template<typename To1>
            static To1 create();

            template<typename To1>
            static one test(To1);

            template<typename>
            static two test(...);
        public:
            static const bool value = (sizeof(test<To>(create<From>())) == sizeof(one));
        };

        template <class T> struct is_integral { static const bool value = false; };
        template <> struct is_integral<bool> { static const bool value = true; };
        template <> struct is_integral<int32_t> { static const bool value = true; };
        template <> struct is_integral<uint32_t> { static const bool value = true; };
        template <> struct is_integral<int64_t> { static const bool value = true; };
        template <> struct is_integral<uint64_t> { static const bool value = true; };
        template <> struct is_integral<float> { static const bool value = true; };
        template <> struct is_integral<double> { static const bool value = true; };

        template <class T>
        struct is_enum {
            static const bool value = is_convertible<T, int32_t>::value & !is_integral<T>::value;
        };

        template <typename T, bool isEnum = is_enum<T>::value> struct isMessage { enum { YES = 1, WRITE_TYPE = WIRETYPE_LENGTH_DELIMITED }; };
        template <typename T> struct isMessage<T, true> { enum { YES = 0, WRITE_TYPE = WIRETYPE_VARINT }; };
        template<> struct isMessage<std::string> { enum { YES = 0, WRITE_TYPE = WIRETYPE_LENGTH_DELIMITED }; };
        template<> struct isMessage<bool> { enum { YES = 0, WRITE_TYPE = WIRETYPE_VARINT }; };
        template<> struct isMessage<int32_t> { enum { YES = 0, WRITE_TYPE = WIRETYPE_VARINT }; };
        template<> struct isMessage<uint32_t> { enum { YES = 0, WRITE_TYPE = WIRETYPE_VARINT }; };
        template<> struct isMessage<int64_t> { enum { YES = 0, WRITE_TYPE = WIRETYPE_VARINT }; };
        template<> struct isMessage<uint64_t> { enum { YES = 0, WRITE_TYPE = WIRETYPE_VARINT }; };
        template<> struct isMessage<float> { enum { YES = 0, WRITE_TYPE = WIRETYPE_32BIT }; };
        template<> struct isMessage<double> { enum { YES = 0, WRITE_TYPE = WIRETYPE_64BIT }; };

        template<class T, class C>
        inline void serialize(T& t, C& c) {
            c.serialize(t);
        }

        template<class T, class C>
        inline void serializeWrapper(T& t, C& c) {
            serialize(t, c);
        }

        template<typename T, bool isEnum = is_enum<T>::value>
        struct TypeTraits {
            typedef T Type;
            static bool isVector() { return false; }
        };

        template<typename T, bool isEnum>
        struct TypeTraits<std::vector<T>, isEnum> {
            typedef std::vector<T> Type;
            static bool isVector() { return true; }
        };

        template<typename T>
        struct TypeTraits<T, true> {
            typedef int32_t Type;
            static bool isVector() { return false; }
        };

    }

}

#endif