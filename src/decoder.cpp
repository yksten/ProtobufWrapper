#include "decoder.h"


namespace proto {

    template <class Dest, class Source>
    inline Dest bit_cast(const Source& source) {
        static_assert(sizeof(Dest) == sizeof(Source), "Sizes do not match");
        Dest dest = Dest();
        memcpy(&dest, &source, sizeof(dest));
        return dest;
    }

    inline bool ConsumeBytes(const uint8_t*& current, size_t how_many, size_t& remaining) {
        if (how_many > remaining) {
            return false;
        }
        current += how_many;
        remaining -= how_many;
        return true;
    }

    template <class T>
    inline bool ReadFromBytes(const uint8_t*& current, size_t& remaining, T& value) {
        bool bConsume = ConsumeBytes(current, sizeof(T), remaining);
        if (bConsume) {
            value = *(bit_cast<T*>(current - sizeof(T)));
        }
        return bConsume;
    }

    bool ReadWireTypeAndFieldNumber(const uint8_t*& current, size_t& remaining, uint8_t& wire_type, uint32_t& field_number) {
        uint8_t wire_type_and_field_number = 0;
        if (!ReadFromBytes(current, remaining, wire_type_and_field_number))
            return false;
        wire_type = wire_type_and_field_number & 0x07;
        field_number = wire_type_and_field_number >> 3;
        if (field_number >= 16) {
            field_number = field_number & 0xf;
            bool keep_going = false;
            do {
                uint8_t next_number = 0;
                if (!ReadFromBytes(current, remaining, next_number))
                    return false;
                keep_going = (next_number >= 128);
                field_number = (field_number << 7) | (next_number & 0x7f);
            } while (keep_going);
        }
        return true;
    }

    /*=====================================message====================================*/

    Message::Message() :_struct(NULL) {
    }

    void Message::setStruct(void* pStruct) {
        _struct = (uint8_t*)pStruct;
    }

    void Message::offset(uint32_t field_number, offset_type n) {
        std::map<uint32_t, converter>::iterator it = _functionSet.find(field_number);
        if (it != _functionSet.end()) {
            it->second.offset(n);
        }
    }

    bool Message::call(uint32_t field_number, const void* cValue) const {
        std::map<uint32_t, converter>::const_iterator it = _functionSet.find(field_number);
        if (it != _functionSet.end()) {
            it->second(_struct, cValue);
            return true;
        }
        return false;
    }

    bool Message::ReadVarInt(const uint8_t*& current, size_t& remaining, uint64_t& result) {
        bool keep_going = false;
        int shift = 0;
        do {
            uint8_t next_number = 0;
            if (!ReadFromBytes(current, remaining, next_number))
                return false;
            keep_going = (next_number >= 128);
            result += (uint64_t)(next_number & 0x7f) << shift;
            shift += 7;
        } while (keep_going);
        return true;
    }

    bool Message::ParseFromBytes(const uint8_t* sz, uint32_t size) {
        const uint8_t* current = sz;
        size_t remaining = size;
        while (remaining > 0) {
            uint8_t wire_type = 0;
            uint32_t field_number = 0;
            if (!ReadWireTypeAndFieldNumber(current, remaining, wire_type, field_number))
                return false;
            switch (wire_type) {
                case serialization::internal::WT_VARINT: {
                    uint64_t value = 0;
                    if (!ReadVarInt(current, remaining, value) || !call(field_number, &value))
                        return false;
                } break;
                case serialization::internal::WT_64BIT: {
                    uint64_t value = 0;
                    if (!ReadFromBytes(current, remaining, value) || !call(field_number, &value))
                        return false;
                } break;
                case serialization::internal::WT_LENGTH_DELIMITED: {
                    uint64_t size = 0;
                    if (!ReadVarInt(current, remaining, size))
                        return false;
                    const uint8_t* data = current;
                    current += size;
                    remaining -= size;
                    bin_type bin(data, size);
                    if (!call(field_number, &bin))
                        return false;
                } break;
                case serialization::internal::WT_GROUP_START:
                case serialization::internal::WT_GROUP_END:
                    return false;
                case serialization::internal::WT_32BIT: {
                    uint32_t value = 0;
                    if (!ReadFromBytes(current, remaining, value) || !call(field_number, &value))
                        return false;
                } break;
                default: {
                    return false;
                } break;
            }
        }
        return true;
    }

}  // namespace proto

namespace serialization {

    PBDecoder::PBDecoder(const uint8_t* sz, unsigned int size)
        : _msg(NULL), _sz(sz), _size(size), _bParseResult(true) {
    }

    bool PBDecoder::decodeValue(serializeItem<bool>& v) {
        return _msg->bind<uint64_t, bool>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeValue(serializeItem<int32_t>& v) {
        return _msg->bind<uint64_t, int32_t>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeValue(serializeItem<uint32_t>& v) {
        return _msg->bind<uint64_t, uint32_t>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeValue(serializeItem<int64_t>& v) {
        return _msg->bind<uint64_t, int64_t>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeValue(serializeItem<uint64_t>& v) {
        return _msg->bind<uint64_t, uint64_t>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeValue(serializeItem<float>& v) {
        return _msg->bind<uint32_t, float>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeValue(serializeItem<double>& v) {
        return _msg->bind<uint64_t, double>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeValue(serializeItem<std::string>& v) {
        return _msg->bind<proto::bin_type, std::string>(&PBDecoder::convertValue, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<bool> >& v) {
        if (v.type >> 16)
            return _msg->bind<proto::bin_type, std::vector<bool> >(&PBDecoder::convertArray, v);
        else
            return _msg->bind<uint64_t, std::vector<bool> >(&PBDecoder::convertArray, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<int32_t> >& v) {
        if (v.type >> 16)
            return _msg->bind<proto::bin_type, std::vector<int32_t> >(&PBDecoder::convertArray, v);
        else
            return _msg->bind<uint64_t, std::vector<int32_t> >(&PBDecoder::convertArray, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<uint32_t> >& v) {
        if (v.type >> 16)
            return _msg->bind<proto::bin_type, std::vector<uint32_t> >(&PBDecoder::convertArray, v);
        else
            return _msg->bind<uint64_t, std::vector<uint32_t> >(&PBDecoder::convertArray, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<int64_t> >& v) {
        if (v.type >> 16)
            return _msg->bind<proto::bin_type, std::vector<int64_t> >(&PBDecoder::convertArray, v);
        else
            return _msg->bind<uint64_t, std::vector<int64_t> >(&PBDecoder::convertArray, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<uint64_t> >& v) {
        if (v.type >> 16)
            return _msg->bind<proto::bin_type, std::vector<uint64_t> >(&PBDecoder::convertArray, v);
        else
            return _msg->bind<uint64_t, std::vector<uint64_t> >(&PBDecoder::convertArray, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<float> >& v) {
        return _msg->bind<uint64_t, std::vector<float> >(&PBDecoder::convertArray, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<double> >& v) {
        return _msg->bind<uint64_t, std::vector<double> >(&PBDecoder::convertArray, v);
    }

    bool PBDecoder::decodeRepaeted(serializeItem<std::vector<std::string> >& v) {
        return _msg->bind<proto::bin_type, std::vector<std::string> >(&PBDecoder::convertArray, v);
    }

}
