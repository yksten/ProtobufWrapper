#ifndef __PB_ENCODER_H__
#define __PB_ENCODER_H__
#include <string>
#include <vector>
#include "serialize.h"


namespace serialization {
    template<typename T>
    struct TypeTraits {
        typedef T Type;
        static bool isVector() { return false; }
    };

    template<typename T>
    struct TypeTraits<std::vector<T> > {
        typedef std::vector<T> Type;
        static bool isVector() { return true; }
    };

    class PBEncoder {
        typedef void(PBEncoder::*writeValue)(uint64_t);
        static writeValue const functionArray[FN_MAX];
        template <int isStruct>
        struct valueEncoder {
            template <typename IN>
            static void encode(const IN& in, int32_t type, PBEncoder& encoder) {
                encoder.encodeValue(in, type);
            }
        };
        template <>
        struct valueEncoder<1> {
            template <typename IN>
            static void encode(const IN& in, int32_t type, PBEncoder& encoder) {
                encoder.operator<<(in);
            }
        };
        friend struct valueEncoder<1>;
        std::string _str;
    public:
        PBEncoder();
        ~PBEncoder();

        const char* data()const;
        uint32_t size()const;

        template<typename T>
        PBEncoder& operator<<(const T& value) {
            T* pValue = const_cast<T*>(&value);
            serializeWrapper(*this, *pValue);
            return *this;
        }

        template<typename T>
        PBEncoder& operator&(const serializePair<T>& pair) {
            if (!isMessage<T>::YES) {
                uint64_t tag = ((uint64_t)pair.tag() << 3) | isMessage<T>::WRITE_TYPE;
                varInt(tag);
                return encodeValue(pair.value(), pair.type());
            }
            uint64_t tag = ((uint64_t)pair.tag() << 3) | WT_LENGTH_DELIMITED;
            varInt(tag);
            std::string strTemp;
            strTemp.swap(_str);
            valueEncoder<isMessage<T>::YES>::encode(pair.value(), pair.type(), *this);
            varInt(_str.length());
            strTemp.append(_str);
            _str.swap(strTemp);
            return *this;
        }

        template<typename T>
        PBEncoder& operator&(const serializePair<std::vector<T> >& pair) {
            uint64_t tag = ((uint64_t)pair.tag() << 3) | WT_LENGTH_DELIMITED;
            uint32_t size = pair.value().size();
            for (uint32_t i = 0; i < size; ++i) {
                varInt(tag);
                std::string strTemp;
                strTemp.swap(_str);
                valueEncoder<isMessage<T>::YES>::encode(pair.value().at(i), pair.type(), *this);
                _str.swap(strTemp);
                varInt(strTemp.length());
                _str.append(strTemp);
            }
            return *this;
        }
    private:
        PBEncoder& encodeValue(const std::string& v, int32_t type);
        PBEncoder& encodeValue(const float& v, int32_t type);
        PBEncoder& encodeValue(const double& v, int32_t type);
        template<typename T>
        PBEncoder& encodeValue(const T& v, int32_t type) {
            value(v, type);
            return *this;
        }
        void value(uint64_t value, int32_t type);
        void varInt(uint64_t value);
        void svarInt(uint64_t value);
        void fixed32(uint64_t value);
        void fixed64(uint64_t value);
        void encodeVarint32(uint32_t low, uint32_t high);
    };
}


#endif