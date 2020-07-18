#include "decoder.h"
#include "thirdParty/picoproto.h"

namespace serialization {

    PBDecoder::PBDecoder(const char* sz, unsigned int size)
        : _rootMsg(new picoproto::Message)
        , _curMsg(_rootMsg)
        , _bParseRet(_rootMsg->ParseFromBytes((uint8_t*)sz, size)) {
    }

    PBDecoder::~PBDecoder() {
        if (_rootMsg)
            delete _rootMsg;
    }

    std::vector<picoproto::Message*> PBDecoder::getMessageArray(int32_t number) {
        return _curMsg->GetMessageArray(number);
    }

    template<>
    void PBDecoder::decodeValue(serializePair<bool>& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetBool(v.num());
    }

    template<>
    void PBDecoder::decodeValue(serializePair<int32_t>& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_SVARINT) {
            v.value() = _curMsg->GetInt32(v.num());
        } else if (v.type() == TYPE_FIXED32) {
            v.value() = (int32_t)_curMsg->GetUInt32(v.num());
        } else {
            v.value() = (int32_t)_curMsg->GetUInt64(v.num());
        }
    }

    template<>
    void PBDecoder::decodeValue(serializePair<uint32_t>& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_FIXED32) {
            v.value() = _curMsg->GetUInt32(v.num());
        } else {
            v.value() = (uint32_t)_curMsg->GetUInt64(v.num());
        }
    }

    template<>
    void PBDecoder::decodeValue(serializePair<int64_t>& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_SVARINT) {
            v.value() = _curMsg->GetInt64(v.num());
        } else {
            v.value() = (int64_t)_curMsg->GetUInt64(v.num());
        }
    }

    template<>
    void PBDecoder::decodeValue(serializePair<uint64_t>& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetUInt64(v.num());
    }

    template<>
    void PBDecoder::decodeValue(serializePair<float>& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetFloat(v.num());
    }

    template<>
    void PBDecoder::decodeValue(serializePair<double>& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetDouble(v.num());
    }

    template<>
    void PBDecoder::decodeValue(serializePair<std::string>& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_BYTES) {
            std::pair<uint8_t*, size_t> temp = _curMsg->GetBytes(v.num());
            v.value().clear();
            v.value().append((const char*)temp.first, temp.second);
        } else {
            v.value() = _curMsg->GetString(v.num());
        }
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<bool> >& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetBoolArray(v.num());
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<int32_t> >& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_SVARINT) {
            v.value() = _curMsg->GetInt32Array(v.num());
        } else if (v.type() == TYPE_FIXED32) {
            std::vector<uint32_t > temp = _curMsg->GetUInt32Array(v.num());
            v.value().assign(temp.begin(), temp.end());
        } else {
            std::vector<uint64_t > temp = _curMsg->GetUInt64Array(v.num());
            v.value().assign(temp.begin(), temp.end());
        }
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<uint32_t> >& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_FIXED32) {
            std::vector<uint32_t > temp = _curMsg->GetUInt32Array(v.num());
            v.value().assign(temp.begin(), temp.end());
        } else {
            std::vector<uint64_t > temp = _curMsg->GetUInt64Array(v.num());
            v.value().assign(temp.begin(), temp.end());
        }
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<int64_t> >& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_SVARINT) {
            v.value() = _curMsg->GetInt64Array(v.num());
        } else {
            std::vector<uint64_t > temp = _curMsg->GetUInt64Array(v.num());
            v.value().assign(temp.begin(), temp.end());
        }
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<uint64_t> >& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetUInt64Array(v.num());
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<float> >& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetFloatArray(v.num());
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<double> >& v) {
        if (!_curMsg) return;

        v.value() = _curMsg->GetDoubleArray(v.num());
    }

    template<>
    void PBDecoder::decodeRepaeted(serializePair<std::vector<std::string> >& v) {
        if (!_curMsg) return;

        if (v.type() == TYPE_BYTES) {
            std::vector<std::pair<uint8_t*, size_t> > temp = _curMsg->GetByteArray(v.num());
            for (uint32_t idx = 0; idx < temp.size(); ++idx) {
                const std::pair<uint8_t*, size_t>& item = temp.at(idx);
                v.value().push_back(std::string((const char*)(item.first), item.second));
            }
        } else {
            v.value() = _curMsg->GetStringArray(v.num());
        }
    }

}
