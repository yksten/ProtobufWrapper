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
        template <int isStruct>
        struct valueEncoder {
            template <typename IN>
            static void encode(const IN& in, PBEncoder& encoder) {
                encoder.encodeValue(in);
            }
        };
        template <>
        struct valueEncoder<1> {
            template <typename IN>
            static void encode(const IN& in, PBEncoder& encoder) {
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
            typename T* pValue = const_cast<T*>(&value);
            serializeWrapper(*this, *pValue);
            return *this;
        }

        template<typename T>
        PBEncoder& operator&(const serializePair<T>& pair) {
            if (!isMessage<T>::YES) {
                uint64_t tag = ((uint64_t)pair.tag() << 3) | isMessage<T>::WRITE_TYPE;
                value(tag);
                return encodeValue(pair);
            }
            uint64_t tag = ((uint64_t)pair.tag() << 3) | WT_LENGTH_DELIMITED;
            value(tag);
            std::string strTemp;
            strTemp.swap(_str);
            valueEncoder<isMessage<T>::YES>::encode(pair.value(), *this);
            value(_str.length());
            strTemp.append(_str);
            _str.swap(strTemp);
            return *this;
        }

        template<typename T>
        PBEncoder& operator&(const serializePair<std::vector<T> >& pair) {
            uint64_t tag = ((uint64_t)pair.tag() << 3) | WT_LENGTH_DELIMITED;
            uint32_t size = pair.value().size();
            for (uint32_t i = 0; i < size; ++i) {
                value(tag);
                std::string strTemp;
                strTemp.swap(_str);
                valueEncoder<isMessage<T>::YES>::encode(pair.value().at(i), *this);
                _str.swap(strTemp);
                value(strTemp.length());
                _str.append(strTemp);
            }
            return *this;
        }
    private:
        template<typename T>
        void encodeValue(const T& v) {
            value(v);
        }

        void encodeValue(const std::string& v);
        void encodeValue(const float& v);
        void encodeValue(const double& v);

        template<typename T>
        PBEncoder& encodeValue(const serializePair<T>& pair) {
            encodeValue(pair.value());
            return *this;
        }
        void value(uint64_t value);
        void encodeVarint32(uint32_t low, uint32_t high);
    };
}


#endif