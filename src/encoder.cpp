#include "encoder.h"

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
	void(*convertSet[])(const uint64_t&, BufferWrapper&) = { &PBEncoder::varInt, &PBEncoder::svarInt, &PBEncoder::fixed32, &PBEncoder::fixed64 };

    PBEncoder::PBEncoder(BufferWrapper& buffer) :_buffer(buffer) {
    }

    PBEncoder::~PBEncoder() {
    }
		
    void PBEncoder::varInt(const uint64_t& value, BufferWrapper& buf) {
        if (value <= 0x7F) {
            char byte = (char)value;
			buf.append(&((char)byte), 1);
        } else {
            encodeVarint((uint32_t)value, (uint32_t)(value >> 32), buf);
        }
    }

    void PBEncoder::svarInt(const uint64_t& value, BufferWrapper& buf) {
        uint64_t zigzagged;
        if (value < 0)
            zigzagged = ~((uint64_t)value << 1);
        else
            zigzagged = (uint64_t)value << 1;
        varInt(zigzagged, buf);
    }

    void PBEncoder::fixed32(const uint64_t& value, BufferWrapper& buf) {
        uint32_t val = static_cast<uint32_t>(value);
		uint8_t bytes[4] = { 0 };
		bytes[0] = (uint8_t)(value & 0xFF);
		bytes[1] = (uint8_t)((value >> 8) & 0xFF);
		bytes[2] = (uint8_t)((value >> 16) & 0xFF);
		bytes[3] = (uint8_t)((value >> 24) & 0xFF);
		buf.append(bytes, 4);
    }

    void PBEncoder::fixed64(const uint64_t& value, BufferWrapper& buf) {
		static uint8_t bytes[8] = { 0 }; 
		bytes[0] = (uint8_t)(value & 0xFF);
		bytes[1] = (uint8_t)((value >> 8) & 0xFF);
        bytes[2] = (uint8_t)((value >> 16) & 0xFF);
		bytes[3] = (uint8_t)((value >> 24) & 0xFF);
        bytes[4] = (uint8_t)((value >> 32) & 0xFF);
		bytes[5] = (uint8_t)((value >> 40) & 0xFF),
        bytes[6] = (uint8_t)((value >> 48) & 0xFF);
		bytes[7] = (uint8_t)((value >> 56) & 0xFF);
		buf.append(bytes, 8);
    }

    void PBEncoder::encodeVarint(uint32_t low, uint32_t high, BufferWrapper& buf) {
        size_t i = 0;
        uint8_t buffer[10] = { 0 };
		uint8_t byte = (uint8_t)(low & 0x7F);
        low >>= 7;

        while (i < 4 && (low != 0 || high != 0)) {
            byte |= 0x80;
            buffer[i++] = byte;
            byte = (uint8_t)(low & 0x7F);
            low >>= 7;
        }

        if (high) {
            byte = (uint8_t)(byte | ((high & 0x07) << 4));
            high >>= 3;

            while (high) {
                byte |= 0x80;
                buffer[i++] = byte;
                byte = (uint8_t)(high & 0x7F);
                high >>= 7;
            }
        }
        buffer[i++] = byte;
        buf.append(buffer, i);
    }

	PBEncoder::enclosure_type PBEncoder::encodeVarint(uint64_t tag, uint32_t type, bool* pHas) {
		enclosure_type info(type, 0, pHas);
		
		uint64_t low = (uint32_t)tag, high = (uint32_t)(tag >> 32);

		size_t i = 0;
		uint8_t byte = (uint8_t)(low & 0x7F);
		low >>= 7;

		while (i < 4 && (low != 0 || high != 0)) {
			byte |= 0x80;
			info.sz[i++] = byte;
			byte = (uint8_t)(low & 0x7F);
			low >>= 7;
		}

		if (high) {
			byte = (uint8_t)(byte | ((high & 0x07) << 4));
			high >>= 3;

			while (high) {
				byte |= 0x80;
				info.sz[i++] = byte;
				byte = (uint8_t)(high & 0x7F);
				high >>= 7;
			}
		}
		info.sz[i++] = byte;
		info.size = i;

		return info;
	}

	void PBEncoder::encodeValue(const bool& v, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !v) {
			buf.append(info.sz, info.size);
			convertSet[info.type](v, buf);
		}
	}

	void PBEncoder::encodeValue(const int32_t& v, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !v) {
			buf.append(info.sz, info.size);
			convertSet[info.type](v, buf);
		}
	}

	void PBEncoder::encodeValue(const uint32_t& v, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !v) {
			buf.append(info.sz, info.size);
			convertSet[info.type](v, buf);
		}
	}

	void PBEncoder::encodeValue(const int64_t& v, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !v) {
			buf.append(info.sz, info.size);
			convertSet[info.type](v, buf);
		}
	}

	void PBEncoder::encodeValue(const uint64_t& v, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !v) {
			buf.append(info.sz, info.size);
			convertSet[info.type](v, buf);
		}
	}

	void PBEncoder::encodeValue(const float& value, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !value) {
			buf.append(info.sz, info.size);
			union { float f; uint32_t i; };
			f = value;
			fixed32(i, buf);
		}
	}

	void PBEncoder::encodeValue(const double& value, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !value) {
			buf.append(info.sz, info.size);
			union { double db; uint64_t i; };
			db = value;
			fixed64(i, buf);
		}
	}

	void PBEncoder::encodeValue(const std::string& v, const enclosure_type& info, BufferWrapper& buf) {
		if (!info.pHas || !v.empty()) {
			buf.append(info.sz, info.size);
			varInt(v.length(), buf);
			buf.append(v.c_str(), v.length());
		}
	}

}
