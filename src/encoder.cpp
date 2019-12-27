#include "encoder.h"

namespace serialization {
    PBEncoder::PBEncoder() {
    }

    PBEncoder::~PBEncoder() {
    }

    const char* PBEncoder::data()const {
        return _str.c_str();
    }

    uint32_t PBEncoder::size()const {
        return _str.size();
    }

    PBEncoder& PBEncoder::encodeValue(const std::string& v) {
        value(v.length());
        _str.append(v);
        return *this;
    }

    PBEncoder& PBEncoder::encodeValue(const float& v) {
        union { float f; uint32_t i; };
        f = v;
        uint8_t bytes[4] = { 0 };
        bytes[0] = (uint8_t)(i & 0xFF);
        bytes[1] = (uint8_t)((i >> 8) & 0xFF);
        bytes[2] = (uint8_t)((i >> 16) & 0xFF);
        bytes[3] = (uint8_t)((i >> 24) & 0xFF);
        _str.append((const char*)bytes, 4);
        return *this;
    }

    PBEncoder& PBEncoder::encodeValue(const double& v) {
        union { double db; uint64_t i; };
        db = v;
        uint8_t bytes[8] = { 0 };
        bytes[0] = (uint8_t)(i & 0xFF);
        bytes[1] = (uint8_t)((i >> 8) & 0xFF);
        bytes[2] = (uint8_t)((i >> 16) & 0xFF);
        bytes[3] = (uint8_t)((i >> 24) & 0xFF);
        bytes[4] = (uint8_t)((i >> 32) & 0xFF);
        bytes[5] = (uint8_t)((i >> 40) & 0xFF);
        bytes[6] = (uint8_t)((i >> 48) & 0xFF);
        bytes[7] = (uint8_t)((i >> 56) & 0xFF);
        _str.append((const char*)bytes, 8);
        return *this;
    }

    void PBEncoder::value(uint64_t value) {
        if (value <= 0x7F) {
            char byte = (char)value;
            _str.append(1, byte);
        } else {
            encodeVarint32((uint32_t)value, (uint32_t)(value >> 32));
        }
    }

    void PBEncoder::encodeVarint32(uint32_t low, uint32_t high) {
        size_t i = 0;
        char buffer[10] = { 0 };
        char byte = (char)(low & 0x7F);
        low >>= 7;

        while (i < 4 && (low != 0 || high != 0)) {
            byte |= 0x80;
            buffer[i++] = byte;
            byte = (char)(low & 0x7F);
            low >>= 7;
        }

        if (high) {
            byte = (char)(byte | ((high & 0x07) << 4));
            high >>= 3;

            while (high) {
                byte |= 0x80;
                buffer[i++] = byte;
                byte = (char)(high & 0x7F);
                high >>= 7;
            }
        }
        buffer[i++] = byte;
        _str.append(buffer, i);
    }
}
