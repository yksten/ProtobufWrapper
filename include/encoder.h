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

		class EXPORTAPI BufferWrapper {
			std::string& _buffer;

			bool _bCalculateFlag;
			size_t _cumtomFieldSize;

			friend class PBEncoder;
		public:
			explicit BufferWrapper(std::string& buffer);

			uint8_t* data() { return (uint8_t*)&_buffer.begin(); }
			const uint8_t* data() const { return (uint8_t*)&_buffer.begin(); }
			size_t size() const { return _buffer.size(); }

			void append(const void* data, size_t len);

			void startCalculateSize() { _bCalculateFlag = true; _cumtomFieldSize = 0; }
			std::pair<bool, size_t> getCustomField() const { return std::pair<bool, size_t>(_bCalculateFlag, _cumtomFieldSize); }
			void setCustomField(const std::pair<bool, size_t>& pair) { _bCalculateFlag = pair.first; _cumtomFieldSize = pair.second; }
		};

    class EXPORTAPI PBEncoder {

		class convertMgr {
			typedef void(*convert_t)(const void*, uint32_t, uint64_t, BufferWrapper&);
			typedef size_t offset_type;
			class converter {
				convert_t _func;
				uint64_t _tag;
				uint32_t _type;
				offset_type _offset;
				bool* _pHas;
			public:
				converter(convert_t func, uint64_t tag, uint32_t type, offset_type offset, bool* pHas)
					:_func(func), _tag(tag), _type(type), _offset(offset), _pHas(pHas) {}
				void operator()(const void* cValue, BufferWrapper& buf) const {
					//has empty
					PBEncoder::varInt(_tag, buf);
					(*_func)(cValue, _type, _tag, buf);
				}
				offset_type offset() const { return _offset; }
				void offset(offset_type offset) { _offset = offset; }
			};
			const uint8_t* _struct;
			std::vector<converter> _functionSet;
		public:
			void setStruct(const void* pStruct) { _struct = (const uint8_t*)pStruct; }
			template<typename T>
			void bind(void(*f)(const T&, uint32_t, uint64_t, BufferWrapper&), const serializeItem<T>& value) {
				uint64_t tag = ((uint64_t)value.num << 3) | internal::isMessage<T>::WRITE_TYPE;
				offset_type offset = ((const uint8_t*)(&value.value)) - _struct;
				_functionSet.push_back(converter(convert_t(f), tag, value.type, offset, value.bHas));
			}
			template<typename T>
			void bind(void(*f)(const std::vector<T>&, uint32_t, uint64_t, BufferWrapper&), const serializeItem<std::vector<T> >& value) {
				uint64_t tag = ((uint64_t)value.num << 3) | internal::isMessage<T>::WRITE_TYPE;
				offset_type offset = ((const uint8_t*)(&value.value)) - _struct;
				_functionSet.push_back(converter(convert_t(f), tag, value.type, offset, value.bHas));
			}
			template<typename K, typename V>
			void bind(void(*f)(const std::map<K, V>&, uint32_t, uint64_t, BufferWrapper&), const serializeItem<std::map<K, V> >& value) {
				uint64_t tag = ((uint64_t)value.num << 3) | internal::WIRETYPE_LENGTH_DELIMITED;
				offset_type offset = ((const uint8_t*)(&value.value)) - _struct;
				_functionSet.push_back(converter(convert_t(f), tag, value.type, offset, value.bHas));
			}
			void doConvert(const uint8_t* pStruct, BufferWrapper& buf) {
				for (uint32_t idx = 0; idx < _functionSet.size(); ++idx) {
					converter& item = _functionSet[idx];
					(item)(pStruct + item.offset(), buf);
				}
			}
		};

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
        BufferWrapper& _buffer;
		convertMgr* _mgr;
    public:
        explicit PBEncoder(BufferWrapper& buffer);
        ~PBEncoder();

		template<typename T>
		PBEncoder::convertMgr& getMessage(T& value) {
			static convertMgr mgr;
			_mgr = &mgr;
			mgr.setStruct(&value);
			internal::serializeWrapper(*this, value);
			return mgr;
		}

        template<typename T>
        bool operator<<(const T& value) {
			static convertMgr& mgr = getMessage(*const_cast<T*>(&value));
			_mgr = &mgr;
			mgr.doConvert((const uint8_t*)&value, _buffer);
			return true;
        }

        template<typename T>
        PBEncoder& operator&(const serializeItem<T>& value) {
			_mgr->bind<internal::TypeTraits<T>::Type>(&PBEncoder::encodeValue, *(const typename internal::TypeTraits<serializeItem<T> >::Type*)(&value));
            return *this;
        }

        template<typename T>
        PBEncoder& operator&(const serializeItem<std::vector<T> >& value) {
            _mgr->bind<internal::TypeTraits<T>::Type>(&PBEncoder::encodeValue, *(const serializeItem<std::vector<typename internal::TypeTraits<T>::Type> >*)(&value));
            return *this;
        }

        template<typename K, typename V>
        PBEncoder& operator&(const serializeItem<std::map<K, V> >& value) {
			_mgr->bind(&PBEncoder::encodeValue, value);
            return *this;
        }
        template<typename V> PBEncoder& operator&(const serializeItem<std::map<float, V> >& value);
        template<typename V> PBEncoder& operator&(const serializeItem<std::map<double, V> >& value);
    private:
        void value(uint64_t value, int32_t type);
        void encodeVarint32(uint32_t low, uint32_t high);
        static void varInt(uint64_t value, BufferWrapper& buf);
        static void svarInt(uint64_t value, BufferWrapper& buf);
        static void fixed32(uint64_t value, BufferWrapper& buf);
        static void fixed64(uint64_t value, BufferWrapper& buf);

		static void encodeValue(const bool& v, uint32_t type, uint64_t tag, BufferWrapper& buf);
		static void encodeValue(const int32_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf);
		static void encodeValue(const uint32_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf);
		static void encodeValue(const int64_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf);
		static void encodeValue(const uint64_t& v, uint32_t type, uint64_t tag, BufferWrapper& buf);
		static void encodeValue(const float& value, uint32_t type, uint64_t tag, BufferWrapper& buf);
		static void encodeValue(const double& value, uint32_t type, uint64_t tag, BufferWrapper& buf);
		static void encodeValue(const std::string& v, uint32_t type, uint64_t tag, BufferWrapper& buf);

		template<typename T>
		static void encodeValue(const T& v, uint32_t type, uint64_t tag, BufferWrapper& buf) {
			size_t nCustomFieldSize = 0;
			PBEncoder encoder(buf);
			do {
				calculateFieldHelper h(buf, nCustomFieldSize);
				encoder << v;
			} while (0);
			varInt(nCustomFieldSize, buf);
			encoder << v;
		}

		template<typename T>
		static void encodeValue(const std::vector<T>& value, uint32_t type, uint64_t tag, BufferWrapper& buf) {
			//uint64_t tag = ((uint64_t)value.num << 3) | internal::WIRETYPE_LENGTH_DELIMITED;
			//varInt(tag);
			//uint32_t size = (uint32_t)value.value.size();
			//for (uint32_t i = 0; i < size; ++i) {
			//	encodeValue(*(const typename internal::TypeTraits<T>::Type*)(&value.value.at(i)), serialization::TYPE_VARINT);
			//}


			uint32_t size = (uint32_t)value.size();
			for (uint32_t i = 0; i < size; varInt(tag, buf), ++i) {
				encodeValue(*(const typename internal::TypeTraits<T>::Type*)(&value.at(i)), type, tag, buf);
			}
		}

		template<typename K, typename V>
		static void encodeValue(const std::map<K, V>& value, uint32_t type, uint64_t tag, BufferWrapper& buf) {
			for (typename std::map<K, V>::const_iterator it = value.begin(); it != value.end(); varInt(tag, buf), ++it) {
				size_t nCustomFieldSize = 0;
				do {
					calculateFieldHelper h(buf, nCustomFieldSize);
					varInt(((uint64_t)1 << 3) | internal::isMessage<K>::WRITE_TYPE, buf);
					encodeValue(*(const typename internal::TypeTraits<K>::Type*)(&it->first), type, tag, buf);
					varInt(((uint64_t)2 << 3) | internal::isMessage<V>::WRITE_TYPE, buf);
					encodeValue(*(const typename internal::TypeTraits<V>::Type*)(&it->second), type, tag, buf);
				} while (0);
				varInt(nCustomFieldSize, buf);

				varInt(((uint64_t)1 << 3) | internal::isMessage<K>::WRITE_TYPE, buf);
				encodeValue(*(const typename internal::TypeTraits<K>::Type*)(&it->first), type, tag, buf);
				varInt(((uint64_t)2 << 3) | internal::isMessage<V>::WRITE_TYPE, buf);
				encodeValue(*(const typename internal::TypeTraits<V>::Type*)(&it->second), type, tag, buf);
			}
		}
    };

}


#endif