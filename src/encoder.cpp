#include "encoder.h"
#include <assert.h>

namespace serialization {

    BufferWrapper::BufferWrapper() : _bCalculateFlag(false), _cumtomFieldSize(0) {
    }

    const uint8_t* BufferWrapper::data() const {
        return (const uint8_t*)_buffer.c_str();
    }

    size_t BufferWrapper::size() const {
        return _buffer.size();
    }

    void BufferWrapper::append(const void* data, size_t len) {
        if (_bCalculateFlag) {
            _cumtomFieldSize += len;
        }
        else {
            _buffer.append((const char*)data, len);
        }
    }

    void BufferWrapper::startCalculateSize() {
        _bCalculateFlag = true;
        _cumtomFieldSize = 0;
    }

    std::pair<bool, size_t> BufferWrapper::getCustomField() const {
        return std::pair<bool, size_t>(_bCalculateFlag, _cumtomFieldSize);
    }

    void BufferWrapper::setCustomField(const std::pair<bool, size_t>& pair) {
        _bCalculateFlag = pair.first;
        _cumtomFieldSize = pair.second;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////

    PBEncoder::PBEncoder(BufferWrapper& buffer) :_buffer(buffer) {
    }

    PBEncoder::~PBEncoder() {
    }

    void PBEncoder::encodeField(const serializeItem<bool>& value) {
        assert(value.type == TYPE_VARINT);
        _mgr->bindValue(&PBEncoder::encodeValue, value);
    }

    void PBEncoder::encodeField(const serializeItem<int32_t>& value) {
        if (value.type == TYPE_VARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueVarInt, *(const serializeItem<uint32_t>*)(&value));
        }
        else if (value.type == TYPE_SVARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueSvarInt, *(const serializeItem<uint32_t>*)(&value));
        }
        else if (value.type == TYPE_FIXED32) {
            _mgr->bindValue(&PBEncoder::encodeValueFixed32, *(const serializeItem<uint32_t>*)(&value));
        }
    }

    void PBEncoder::encodeField(const serializeItem<uint32_t>& value) {
        if (value.type == TYPE_VARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueVarInt, value);
        }
        else if (value.type == TYPE_SVARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueSvarInt, value);
        }
        else if (value.type == TYPE_FIXED32) {
            _mgr->bindValue(&PBEncoder::encodeValueFixed32, value);
        }
    }

    void PBEncoder::encodeField(const serializeItem<int64_t>& value) {
        if (value.type == TYPE_VARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueVarInt, *(const serializeItem<uint64_t>*)(&value));
        }
        else if (value.type == TYPE_SVARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueSvarInt, *(const serializeItem<uint64_t>*)(&value));
        }
        else if (value.type == TYPE_FIXED32) {
            _mgr->bindValue(&PBEncoder::encodeValueFixed64, *(const serializeItem<uint64_t>*)(&value));
        }
    }

    void PBEncoder::encodeField(const serializeItem<uint64_t>& value) {
        if (value.type == TYPE_VARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueVarInt, value);
        }
        else if (value.type == TYPE_SVARINT) {
            _mgr->bindValue(&PBEncoder::encodeValueSvarInt, value);
        }
        else if (value.type == TYPE_FIXED32) {
            _mgr->bindValue(&PBEncoder::encodeValueFixed64, value);
        }
    }

    void PBEncoder::encodeField(const serializeItem<float>& value) {
        _mgr->bindValue(&PBEncoder::encodeValue, value);
    }

    void PBEncoder::encodeField(const serializeItem<double>& value) {
        _mgr->bindValue(&PBEncoder::encodeValue, value);
    }

    void PBEncoder::encodeField(const serializeItem<std::string>& value) {
        _mgr->bindValue(&PBEncoder::encodeValue, value);
    }

    void PBEncoder::varInt(uint64_t value, BufferWrapper& buf) {
        uint8_t i = 0;
        uint8_t buffer[10];
        while (value >= 0x80) {
            buffer[i++] = static_cast<uint8_t>(value | 0x80);
            value >>= 7;
        }
        buffer[i++] = static_cast<uint8_t>(value);
        buf.append(buffer, i);
    }
    
    void PBEncoder::fixed32(uint32_t value, BufferWrapper& buf) {
        uint8_t bytes[4] = { (uint8_t)(value & 0xFF), (uint8_t)((value >> 8) & 0xFF),
            (uint8_t)((value >> 16) & 0xFF), (uint8_t)((value >> 24) & 0xFF) };
        buf.append(bytes, 4);
    }

    void PBEncoder::fixed64(uint64_t value, BufferWrapper& buf) {
        uint8_t bytes[8] = { (uint8_t)(value & 0xFF), (uint8_t)((value >> 8) & 0xFF),
            (uint8_t)((value >> 16) & 0xFF), (uint8_t)((value >> 24) & 0xFF),
            (uint8_t)((value >> 32) & 0xFF), (uint8_t)((value >> 40) & 0xFF),
            (uint8_t)((value >> 48) & 0xFF), (uint8_t)((value >> 56) & 0xFF) };
        buf.append(bytes, 8);
    }

    PBEncoder::enclosure_type PBEncoder::encodeVarint(uint64_t tag, uint32_t type, bool* pHas) {
        enclosure_type info(type, 0, pHas);

        while (tag >= 0x80) {
            info.sz[info.size++] = static_cast<uint8_t>(tag | 0x80);
            tag >>= 7;
        }
        info.sz[info.size++] = static_cast<uint8_t>(tag);

        return info;
    }

    void PBEncoder::encodeValue(const bool& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || v) {
            buf.append(info.sz, info.size);
            uint8_t byte = (uint8_t)v;
            buf.append(&byte, 1);
        }
    }

    void PBEncoder::encodeValue(const int32_t& v, const enclosure_type& info, BufferWrapper& buf) {
        encodeValueVarInt(*reinterpret_cast<const uint32_t*>(&v), info, buf);
    }

    void PBEncoder::encodeValue(const uint32_t& v, const enclosure_type& info, BufferWrapper& buf) {
        encodeValueVarInt(v, info, buf);
    }

    void PBEncoder::encodeValue(const int64_t& v, const enclosure_type& info, BufferWrapper& buf) {
        encodeValueVarInt(*reinterpret_cast<const uint64_t*>(&v), info, buf);
    }

    void PBEncoder::encodeValue(const uint64_t& v, const enclosure_type& info, BufferWrapper& buf) {
        encodeValueVarInt(v, info, buf);
    }

    void PBEncoder::encodeValue(const float& value, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || value) {
            buf.append(info.sz, info.size);
            fixed32(*reinterpret_cast<const uint32_t*>(&value), buf);
        }
    }

    void PBEncoder::encodeValue(const double& value, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || value) {
            buf.append(info.sz, info.size);
            fixed64(*reinterpret_cast<const uint64_t*>(&value), buf);
        }
    }

    void PBEncoder::encodeValue(const std::string& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || !v.empty()) {
            buf.append(info.sz, info.size);
            varInt(v.length(), buf);
            buf.append(v.c_str(), v.length());
        }
    }

    void PBEncoder::encodeValueVarInt(const uint32_t& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || v) {
            buf.append(info.sz, info.size);
            
            uint32_t value = v;
            uint8_t i = 0;
            uint8_t buffer[5];
            while (value >= 0x80) {
                buffer[i++] = static_cast<uint8_t>(value | 0x80);
                value >>= 7;
            }
            buffer[i++] = static_cast<uint8_t>(value);
            buf.append(buffer, i);
        }
    }

    void PBEncoder::encodeValueSvarInt(const uint32_t& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || v) {
            buf.append(info.sz, info.size);
            uint32_t zigzagged;
            if (v < 0)
                zigzagged = ~((uint32_t)v << 1);
            else
                zigzagged = (uint32_t)v << 1;

            uint8_t i = 0;
            uint8_t buffer[5];
            while (zigzagged >= 0x80) {
                buffer[i++] = static_cast<uint8_t>(zigzagged | 0x80);
                zigzagged >>= 7;
            }
            buffer[i++] = static_cast<uint8_t>(zigzagged);
            buf.append(buffer, i);
        }
    }

    void PBEncoder::encodeValueFixed32(const uint32_t& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || v) {
            buf.append(info.sz, info.size);
            uint8_t bytes[4] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF),
                (uint8_t)((v >> 16) & 0xFF), (uint8_t)((v >> 24) & 0xFF) };
            buf.append(bytes, 4);
        }
    }

    void PBEncoder::encodeValueVarInt(const uint64_t& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || v) {
            buf.append(info.sz, info.size);
            
            uint64_t value = v;
            uint8_t i = 0;
            uint8_t buffer[10];
            while (value >= 0x80) {
                buffer[i++] = static_cast<uint8_t>(value | 0x80);
                value >>= 7;
            }
            buffer[i++] = static_cast<uint8_t>(value);
            buf.append(buffer, i);
        }
    }

    void PBEncoder::encodeValueSvarInt(const uint64_t& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || v) {
            buf.append(info.sz, info.size);
            uint64_t zigzagged;
            if (v < 0)
                zigzagged = ~((uint64_t)v << 1);
            else
                zigzagged = (uint64_t)v << 1;
            
            uint8_t i = 0;
            uint8_t buffer[10];
            while (zigzagged >= 0x80) {
                buffer[i++] = static_cast<uint8_t>(zigzagged | 0x80);
                zigzagged >>= 7;
            }
            buffer[i++] = static_cast<uint8_t>(zigzagged);
            buf.append(buffer, i);
        }
    }

    void PBEncoder::encodeValueFixed64(const uint64_t& v, const enclosure_type& info, BufferWrapper& buf) {
        if (!info.pHas || v) {
            buf.append(info.sz, info.size);
            fixed64(v, buf);
        }
    }

}
