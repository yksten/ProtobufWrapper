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

    void PBEncoder::encodeValue(const std::string& v) {
        value(v.length());
        _str.append(v);
    }

    void PBEncoder::encodeValue(const float& v) {
        union {
            float f;
            uint32_t i;
        };
        f = v;
        _str.append((const char*)&i, sizeof(uint32_t));
    }

    void PBEncoder::encodeValue(const double& v) {
        union {
            double db;
            uint64_t i;
        };
        db = v;
        _str.append((const char*)&i, sizeof(uint64_t));
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
