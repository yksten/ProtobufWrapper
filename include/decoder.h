#ifndef __PB_DECODER_H__
#define __PB_DECODER_H__
#include <assert.h>
#include <string>
#include <vector>
#include "serialize.h"

namespace serialization {
    class PBDecoder {
        const char* _szBuf;
        const uint32_t _size;
        mutable uint32_t _cur;

        template <int isStruct>
        struct valueDecoder {
            template <typename OUT>
            static void decode(OUT& out, const PBDecoder& decoder) {
                decoder.operator>>(out);
            }
        };
        template <>
        struct valueDecoder<0> {
            template <typename OUT>
            static void decode(OUT& out, const PBDecoder& decoder) {
                out = static_cast<OUT>(decoder.value());
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
                return decodeValue(pair);
            }
            uint32_t temp = tagWriteType();
            assert(pair.tag() == tag(temp));
            assert(WT_LENGTH_DELIMITED == writeType(temp));
            uint64_t length = value();
            assert(_cur + length <= _size);
            _cur += length;
            return operator&(pair);
        }

        template<typename T>
        const PBDecoder& operator&(serializePair<std::vector<T> > pair) const {
            for (; hasData();) {
                uint32_t cur = _cur;
                uint32_t temp = tagWriteType();
                if (pair.tag() == tag(temp) && WT_LENGTH_DELIMITED == writeType(temp)) {
                    assert(value());
                    T item = T();
                    valueDecoder<isMessage<T>::YES>::decode(item, *this);
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
        const PBDecoder& decodeValue(serializePair<T>& pair) const;
        char readByte()const;
        uint32_t tagWriteType()const;
        uint64_t value()const;
        bool hasData()const { return _cur < _size; }
        uint32_t tag(uint32_t value)const { return (value >> 3); }
        uint32_t writeType(uint32_t value)const { return (value & 7); }
    };
}

#endif