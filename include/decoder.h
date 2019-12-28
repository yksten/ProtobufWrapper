#ifndef __PB_DECODER_H__
#define __PB_DECODER_H__
#include <assert.h>
#include <string>
#include <vector>
#include "serialize.h"

namespace serialization {
    class PBDecoder {
        typedef uint64_t(PBDecoder::*readValue)()const;
        static readValue const functionArray[FN_MAX];
        const char* _szBuf;
        const uint32_t _size;
        mutable uint32_t _cur;

        template <int isStruct>
        struct valueDecoder {
            template <typename OUT>
            static void decode(OUT& out, int32_t typ, const PBDecoder& decoder) {
                decoder.operator>>(out);
            }
        };
        template <>
        struct valueDecoder<0> {
            template <typename OUT>
            static void decode(OUT& out, int32_t type, const PBDecoder& decoder) {
                decoder.decodeValue(out, type);
            }
        };
        friend struct valueDecoder<0>;
    public:
        PBDecoder(const char* sz, uint32_t size);
        ~PBDecoder();

        template<typename T>
        const PBDecoder& operator>>(T& value)const {
            serializeWrapper(*this, value);
            return *this;
        }

        template<typename T>
        const PBDecoder& operator&(serializePair<T> pair) const {
            if (!isMessage<T>::YES) {
                uint32_t temp = tagWriteType();
                assert(pair.tag() == tag(temp));
                assert(isMessage<T>::WRITE_TYPE == writeType(temp));
                return decodeValue(pair.value(), pair.type());
            }
            uint32_t temp = tagWriteType();
            assert(pair.tag() == tag(temp));
            assert(WT_LENGTH_DELIMITED == writeType(temp));
            uint64_t length = varInt();
            assert(_cur + length <= _size);
            _cur += length;
            valueDecoder<isMessage<T>::YES>::decode(pair.value(), pair.type(), *this);
            return *this;
        }

        template<typename T>
        const PBDecoder& operator&(serializePair<std::vector<T> > pair) const {
            if (pair.type() == PACK) {
                return decodeRepaetedPack(pair);
            }
            for (; hasData();) {
                uint32_t cur = _cur;
                uint32_t temp = tagWriteType();
                if (pair.tag() == tag(temp) && WT_LENGTH_DELIMITED == writeType(temp)) {
                    assert(varInt());
                    T item = T();
                    valueDecoder<isMessage<T>::YES>::decode(item, pair.type(), *this);
                    pair.value().push_back(item);
                } else {
                    _cur = cur;
                    break;
                }
            }
            return *this;
        }
    private:
        template<typename T>
        const PBDecoder& decodeRepaetedPack(serializePair<std::vector<T> >& pair) const {
            uint32_t temp = tagWriteType();
            assert(pair.tag() == tag(temp));
            assert(WT_LENGTH_DELIMITED == writeType(temp));
            uint32_t cur = _cur;
            uint64_t size = varInt();
            for (; _cur - cur < size;) {
                T item = T();
                valueDecoder<isMessage<T>::YES>::decode(item, pair.type(), *this);
                pair.value().push_back(item);
            }
            assert((_cur - (cur + 1)) == size);
            return *this;
        }
        const PBDecoder& decodeValue(std::string& v, int32_t type) const;
        const PBDecoder& decodeValue(float& v, int32_t type) const;
        const PBDecoder& decodeValue(double& v, int32_t type) const;
        template<typename T>
        const PBDecoder& decodeValue(T& v, int32_t type) const {
            v = static_cast<T>(value(type));
            return *this;
        }
        char readByte()const;
        uint32_t tagWriteType()const;
        uint64_t value(int32_t type)const;
        uint64_t varInt()const;
        uint64_t svarInt()const;
        uint64_t fixed32()const;
        uint64_t fixed64()const;
        bool hasData()const { return _cur < _size; }
        uint32_t tag(uint32_t value)const { return (value >> 3); }
        uint32_t writeType(uint32_t value)const { return (value & 7); }
    };
}

#endif