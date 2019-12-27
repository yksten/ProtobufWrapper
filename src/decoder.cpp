#include "decoder.h"

namespace serialization {
    PBDecoder::readValue const PBDecoder::functionArray[] = { &PBDecoder::varInt, &PBDecoder::svarInt, &PBDecoder::fixed32, &PBDecoder::fixed64, };
    PBDecoder::PBDecoder(const char* sz, unsigned int size)
        :_szBuf(sz), _size(size), _cur(0) {
    }

    PBDecoder::~PBDecoder() {
        assert(_cur == _size);
    }

    const PBDecoder& PBDecoder::decodeValue(std::string& v, int32_t type) const {
        uint64_t length = varInt();
        assert(_cur + length <= _size);
        v.append(_szBuf + _cur, length);
        _cur += length;
        return *this;
    }

    const PBDecoder& PBDecoder::decodeValue(float& v, int32_t type) const {
        assert(_cur + 4 <= _size);
        uint8_t szTemp[4] = { 0 };
        memcpy(szTemp, _szBuf + _cur, 4);
        union { float f; uint32_t i; };
        i = ((uint32_t)szTemp[0] << 0) | ((uint32_t)szTemp[1] << 8) | ((uint32_t)szTemp[2] << 16) | ((uint32_t)szTemp[3] << 24);
        v = f;
        _cur += 4;
        return *this;
    }

    const PBDecoder& PBDecoder::decodeValue(double& v, int32_t type) const {
        assert(_cur + 8 <= _size);
        uint8_t szTemp[8] = { 0 };
        memcpy(szTemp, _szBuf + _cur, 8);
        union { double db; uint64_t i; };
        i = ((uint64_t)szTemp[0] << 0) | ((uint64_t)szTemp[1] << 8) | ((uint64_t)szTemp[2] << 16) | ((uint64_t)szTemp[3] << 24) | ((uint64_t)szTemp[4] << 32) | ((uint64_t)szTemp[5] << 40) | ((uint64_t)szTemp[6] << 48) | ((uint64_t)szTemp[7] << 56);
        v = db;
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

    uint64_t PBDecoder::value(int32_t type)const {
        return (this->*functionArray[type])();
    }

    uint64_t PBDecoder::varInt()const {
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

    uint64_t PBDecoder::svarInt()const {
        uint64_t value = varInt();
        if (value & 1) {
            return (uint64_t)(~(value >> 1));
        }
        return (uint64_t)(value >> 1);
    }

    uint64_t PBDecoder::fixed32()const {
        assert(_cur + 4 <= _size);
        uint8_t szTemp[4] = { 0 };
        memcpy(szTemp, _szBuf + _cur, 4);
        uint32_t i = ((uint32_t)szTemp[0] << 0) | ((uint32_t)szTemp[1] << 8) | ((uint32_t)szTemp[2] << 16) | ((uint32_t)szTemp[3] << 24);
        _cur += 4;
        return i;
    }

    uint64_t PBDecoder::fixed64()const {
        assert(_cur + 8 <= _size);
        uint8_t szTemp[8] = { 0 };
        memcpy(szTemp, _szBuf + _cur, 8);
        uint64_t i = ((uint64_t)szTemp[0] << 0) | ((uint64_t)szTemp[1] << 8) | ((uint64_t)szTemp[2] << 16) | ((uint64_t)szTemp[3] << 24) | ((uint64_t)szTemp[4] << 32) | ((uint64_t)szTemp[5] << 40) | ((uint64_t)szTemp[6] << 48) | ((uint64_t)szTemp[7] << 56);
        _cur += 8;
        return i;
    }
}
