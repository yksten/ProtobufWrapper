#include "decoder.h"

namespace serialization {
    PBDecoder::PBDecoder(const char* sz, unsigned int size)
        :_szBuf(sz), _size(size), _cur(0) {
    }

    PBDecoder::~PBDecoder() {
        assert(_cur == _size);
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<bool>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<bool>(value());
        return *this;
    }
    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<int8_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<int8_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<uint8_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<uint8_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<int16_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<int16_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<uint16_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<uint16_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<int32_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<int32_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<uint32_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<uint32_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<int64_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<int64_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<uint64_t>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_VARINT == writeType(temp));
        pair.value() = static_cast<uint64_t>(value());
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<std::string>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_LENGTH_DELIMITED == writeType(temp));
        uint64_t length = value();
        assert(_cur + length <= _size);
        pair.value().append(_szBuf + _cur, length);
        _cur += length;
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<float>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_32BIT == writeType(temp));
        assert(_cur + 4 <= _size);
        *(uint32_t*)&pair.value() = ((uint32_t)_szBuf[_cur + 0] << 0) |
                                    ((uint32_t)_szBuf[_cur + 1] << 8) |
                                    ((uint32_t)_szBuf[_cur + 2] << 16) |
                                    ((uint32_t)_szBuf[_cur + 3] << 24);
        _cur += 4;
        return *this;
    }

    template<>
    const PBDecoder& PBDecoder::decodeValue(serializePair<double>& pair) const {
        uint32_t temp = tagWriteType();
        assert(pair.tag() == tag(temp));
        assert(WT_64BIT == writeType(temp));
        assert(_cur + 8 <= _size);
        *(uint64_t*)&pair.value() = ((uint64_t)_szBuf[_cur + 0] << 0) |
                                    ((uint64_t)_szBuf[_cur + 1] << 8) |
                                    ((uint64_t)_szBuf[_cur + 2] << 16) |
                                    ((uint64_t)_szBuf[_cur + 3] << 24) |
                                    ((uint64_t)_szBuf[_cur + 4] << 32) |
                                    ((uint64_t)_szBuf[_cur + 5] << 40) |
                                    ((uint64_t)_szBuf[_cur + 6] << 48) |
                                    ((uint64_t)_szBuf[_cur + 7] << 56);
        _cur += 8;
        return *this;
    }

    char PBDecoder::readByte()const {
        assert(_cur <= _size);
        char ch = _szBuf[_cur];
        ++_cur;
        return ch;
    }

    uint32_t PBDecoder::tagWriteType()const {
        uint32_t result = 0;
        char byte = readByte();
        if (byte & 0x80 == 0) {
            result = byte;
        } else {
            result = byte & 0x7F;
            for (uint8_t bitpos = 7; (byte & 0x80); bitpos = (uint8_t)(bitpos + 7)) {
                byte = readByte();
                if (bitpos >= 32) {
                    /* Note: The varint could have trailing 0x80 bytes, or 0xFF for negative. */
                    uint8_t sign_extension = (bitpos < 63) ? 0xFF : 0x01;
                    if ((byte & 0x7F) != 0x00 && ((result >> 31) == 0 || byte != sign_extension)) {
                        assert(false);//"varint overflow";
                    }
                    continue;
                }
                result |= (uint32_t)(byte & 0x7F) << bitpos;
            }
        }
        return result;
    }

    uint64_t PBDecoder::value()const {
        assert(_cur <= _size);
        char byte = readByte();
        uint64_t result = byte;
        for (uint8_t bitpos = 7; (byte & 0x80); bitpos = (uint8_t)(bitpos + 7)) {
            if (bitpos >= 64)
                assert(false);//"varint overflow";

            byte = readByte();
            result |= (uint64_t)(byte & 0x7F) << bitpos;
        }
        return result;
    }
}
