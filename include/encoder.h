#ifndef __PROTOBUF_ENCODER_H__
#define __PROTOBUF_ENCODER_H__

#include <string>
#include <map>
#include "serialize.h"


namespace serialization {

	template<typename T>
	inline bool empty(const T& v) {
		uint8_t* szValue = (uint8_t*)&v;
		for (uint32_t idx = 0; idx < sizeof(T); ++idx) {
			if (szValue[idx])
				return false;
		}
		return true;
	}
	inline bool empty(const std::string& v) { return v.empty(); }

	class EXPORTAPI BufferWrapper {
		std::string& _buffer;

		bool _bCalculateFlag;
		size_t _cumtomFieldSize;

	public:
		explicit BufferWrapper(std::string& buffer);

		uint8_t* data() { return (uint8_t*)&_buffer.begin(); }
		const uint8_t* data() const { return (uint8_t*)&_buffer.begin(); }
		size_t size() const { return _buffer.size(); }

		void append(const void* data, size_t len);

		void startCalculateSize();
		std::pair<bool, size_t> getCustomField() const { return std::pair<bool, size_t>(_bCalculateFlag, _cumtomFieldSize); }
		void setCustomField(const std::pair<bool, size_t>& pair) { _bCalculateFlag = pair.first; _cumtomFieldSize = pair.second; }
	};

    class EXPORTAPI PBEncoder {
        class calculateFieldHelper {
            BufferWrapper& _buff;
            size_t& _nSize;
            std::pair<bool, size_t> _customField;
        public:
            calculateFieldHelper(BufferWrapper& buff, size_t& nSize)
                :_buff(buff), _nSize(nSize), _customField(_buff.getCustomField()) {
                _buff.startCalculateSize();
            }
            ~calculateFieldHelper() {
                _nSize = _buff.getCustomField().second;
                _buff.setCustomField(_customField);
            }
        };

        typedef void(PBEncoder::*writeValue)(uint64_t);
        static writeValue const functionArray[4];
        BufferWrapper _buffer;
    public:
        explicit PBEncoder(std::string& buffer);
        ~PBEncoder();

        const char* data()const;
        uint32_t size()const;

        template<typename T>
        bool operator<<(const T& value) {
            T* pValue = const_cast<T*>(&value);
            internal::serializeWrapper(*this, *pValue);
            return true;
        }

        template<typename T>
        PBEncoder& operator&(const serializeItem<T>& value) {
			if (!value.bHas || !empty(value.value)) {
				uint64_t tag = ((uint64_t)value.num << 3) | internal::isMessage<T>::WRITE_TYPE;
				varInt(tag);
				if (internal::isMessage<T>::YES) {
					size_t nCustomFieldSize = 0;
					do {
						calculateFieldHelper h(_buffer, nCustomFieldSize);
						encodeValue(*(const typename internal::TypeTraits<T>::Type*)(&value.value), value.type);
					} while (0);
					varInt(nCustomFieldSize);
				}
				encodeValue(*(const typename internal::TypeTraits<T>::Type*)(&value.value), value.type);
			}
            return *this;
        }

        template<typename T>
        PBEncoder& operator&(const serializeItem<std::vector<T> >& value) {
            if (!internal::isMessage<T>::YES && value.type == TYPE_PACK) {
                return encodeRepaetedPack(value);
            }

            uint64_t tag = ((uint64_t)value.num << 3) | internal::isMessage<T>::WRITE_TYPE;
            uint32_t size = (uint32_t)value.value.size();
            for (uint32_t i = 0; i < size; ++i) {
                varInt(tag);
                if (internal::isMessage<T>::YES) {
                    size_t nCustomFieldSize = 0;
                    do {
                        calculateFieldHelper h(_buffer, nCustomFieldSize);
                        encodeValue(*(const typename internal::TypeTraits<T>::Type*)(&value.value.at(i)), value.type);
                    } while (0);
                    varInt(nCustomFieldSize);
                }
                encodeValue(*(const typename internal::TypeTraits<T>::Type*)(&value.value.at(i)), value.type);
            }
            return *this;
        }

        template<typename K, typename V>
        PBEncoder& operator&(const serializeItem<std::map<K, V> >& value) {
            uint64_t tag = ((uint64_t)value.num << 3) | internal::WIRETYPE_LENGTH_DELIMITED;
            for (typename std::map<K, V>::const_iterator it = value.value.begin(); it != value.value.end(); ++it) {
                varInt(tag);

                size_t nCustomFieldSize = 0;
                do {
                    calculateFieldHelper h(_buffer, nCustomFieldSize);
                    varInt(((uint64_t)1 << 3) | internal::isMessage<K>::WRITE_TYPE);
                    encodeValue(*(const typename internal::TypeTraits<K>::Type*)(&it->first), TYPE_VARINT);
                    operator&(SERIALIZE(2, *const_cast<V*>(&it->second)));
                } while (0);
                varInt(nCustomFieldSize);

                varInt(((uint64_t)1 << 3) | internal::isMessage<K>::WRITE_TYPE);
                encodeValue(*(const typename internal::TypeTraits<K>::Type*)(&it->first), TYPE_VARINT);
                operator&(SERIALIZE(2, *const_cast<V*>(&it->second)));
            }
            return *this;
        }
        template<typename V> PBEncoder& operator&(const serializeItem<std::map<float, V> >& value);
        template<typename V> PBEncoder& operator&(const serializeItem<std::map<double, V> >& value);
    private:
        template<typename T>
        PBEncoder& encodeRepaetedPack(const serializeItem<std::vector<T> >& value) {
            uint64_t tag = ((uint64_t)value.num << 3) | internal::WIRETYPE_LENGTH_DELIMITED;
            varInt(tag);
            uint32_t size = (uint32_t)value.value.size();
            for (uint32_t i = 0; i < size; ++i) {
                encodeValue(*(const typename internal::TypeTraits<T>::Type*)(&value.value.at(i)), serialization::TYPE_VARINT);
            }
            return *this;
        }
        template<typename T>
        void encodeValue(const T& v, int32_t type) {
            operator<<(v);
        }
        void encodeValue(const std::string& v, int32_t type);
        void encodeValue(const float& v, int32_t type);
        void encodeValue(const double& v, int32_t type);
        void encodeValue(const bool& v, int32_t type) { value((uint64_t)v, type); }
        void encodeValue(const int32_t& v, int32_t type) { value((uint64_t)v, type); }
        void encodeValue(const uint32_t& v, int32_t type) { value((uint64_t)v, type); }
        void encodeValue(const int64_t& v, int32_t type) { value((uint64_t)v, type); }
        void encodeValue(const uint64_t& v, int32_t type) { value(v, type); }
        void value(uint64_t value, int32_t type);
        void varInt(uint64_t value);
        void svarInt(uint64_t value);
        void fixed32(uint64_t value);
        void fixed64(uint64_t value);
        void encodeVarint32(uint32_t low, uint32_t high);
    };

}


#endif