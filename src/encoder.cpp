#include "encoder.h"

namespace serialization {

	BufferWrapper::BufferWrapper(std::string& buffer) : _buffer(buffer), _bCalculateFlag(false), _cumtomFieldSize(0) {
	}

	void BufferWrapper::append(const void* data, size_t len) {
		if (_bCalculateFlag) {
			_cumtomFieldSize += len;
		}
		else {
			_buffer.append((const char*)data, len);
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////
    PBEncoder::PBEncoder(BufferWrapper& buffer) :_buffer(buffer) {
    }

    PBEncoder::~PBEncoder() {
    }

    void PBEncoder::value(uint64_t value, int32_t type) {
        //(this->*functionArray[type])(value);
    }

    void PBEncoder::varInt(uint64_t value, BufferWrapper& buf) {
        if (value <= 0x7F) {
            char byte = (char)value;
			buf.append(&((char)byte), 1);
        } else {
            //encodeVarint32((uint32_t)value, (uint32_t)(value >> 32));
        }
    }

    void PBEncoder::svarInt(uint64_t value, BufferWrapper& buf) {
        uint64_t zigzagged;
        if (value < 0)
            zigzagged = ~((uint64_t)value << 1);
        else
            zigzagged = (uint64_t)value << 1;
        varInt(zigzagged, buf);
    }

    void PBEncoder::fixed32(uint64_t value, BufferWrapper& buf) {
        uint32_t val = static_cast<uint32_t>(value);
        uint8_t bytes[4] = { (uint8_t)(val & 0xFF), (uint8_t)((val >> 8) & 0xFF),
            (uint8_t)((val >> 16) & 0xFF), (uint8_t)((val >> 24) & 0xFF) };
		buf.append(bytes, 4);
    }

    void PBEncoder::fixed64(uint64_t value, BufferWrapper& buf) {
        uint8_t bytes[8] = { (uint8_t)(value & 0xFF), (uint8_t)((value >> 8) & 0xFF),
            (uint8_t)((value >> 16) & 0xFF), (uint8_t)((value >> 24) & 0xFF),
            (uint8_t)((value >> 32) & 0xFF), (uint8_t)((value >> 40) & 0xFF),
            (uint8_t)((value >> 48) & 0xFF), (uint8_t)((value >> 56) & 0xFF) };
		buf.append(bytes, 8);
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
        _buffer.append(buffer, i);
    }

	void PBEncoder::encodeValue(const bool& v, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		if (type == TYPE_VARINT) {
			varInt(v, buf);
		}
		else if (type == TYPE_SVARINT) {
			svarInt(v, buf);
		}
		else if (type == TYPE_FIXED32) {
			fixed32(v, buf);
		}
		else if (type == TYPE_FIXED64) {
			fixed64(v, buf);
		}
	}

	void PBEncoder::encodeValue(const int32_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		if (type == TYPE_VARINT) {
			varInt(v, buf);
		}
		else if (type == TYPE_SVARINT) {
			svarInt(v, buf);
		}
		else if (type == TYPE_FIXED32) {
			fixed32(v, buf);
		}
		else if (type == TYPE_FIXED64) {
			fixed64(v, buf);
		}
	}

	void PBEncoder::encodeValue(const uint32_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		if (type == TYPE_VARINT) {
			varInt(v, buf);
		}
		else if (type == TYPE_SVARINT) {
			svarInt(v, buf);
		}
		else if (type == TYPE_FIXED32) {
			fixed32(v, buf);
		}
		else if (type == TYPE_FIXED64) {
			fixed64(v, buf);
		}
	}

	void PBEncoder::encodeValue(const int64_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		if (type == TYPE_VARINT) {
			varInt(v, buf);
		}
		else if (type == TYPE_SVARINT) {
			svarInt(v, buf);
		}
		else if (type == TYPE_FIXED32) {
			fixed32(v, buf);
		}
		else if (type == TYPE_FIXED64) {
			fixed64(v, buf);
		}
	}

	void PBEncoder::encodeValue(const uint64_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		if (type == TYPE_VARINT) {
			varInt(v, buf);
		}
		else if (type == TYPE_SVARINT) {
			svarInt(v, buf);
		}
		else if (type == TYPE_FIXED32) {
			fixed32(v, buf);
		}
		else if (type == TYPE_FIXED64) {
			fixed64(v, buf);
		}
	}

	void PBEncoder::encodeValue(const float& value, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		union { float f; uint32_t i; };
		f = value;
		fixed32(i, buf);
	}

	void PBEncoder::encodeValue(const double& value, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		union { double db; uint64_t i; };
		db = value;
		fixed64(i, buf);
	}

	void PBEncoder::encodeValue(const std::string& v, uint32_t type, uint64_t tag, BufferWrapper& buf) {
		varInt(v.length(), buf);
		buf.append(v.c_str(), v.size());
	}

}
