#ifndef __PROTOBUF_DECODER_H__
#define __PROTOBUF_DECODER_H__
#include <assert.h>
#include <string>
#include <vector>
#include "serialize.h"

namespace proto {
    class Message;
}

namespace serialization {

    class PBDecoder {
        proto::Message* _rootMsg;
        proto::Message* _curMsg;
        bool _bParseRet;

        template <int isStruct>
        struct valueDecoder {
            template <typename OUT>
            static void decode(serializePair<OUT>& out, PBDecoder& decoder) {
                proto::Message* msg = decoder.getCurMsg(); {
                    decoder.setCurMsg(decoder.getMessage(out.num()));
                    decoder.operator>>(out.value());
                } decoder.setCurMsg(msg);
            }

            template <typename T>
            static void decodeRepaeted(serializePair<std::vector<T> >& out, PBDecoder& decoder) {
                proto::Message* msg = decoder.getCurMsg();
                std::vector<proto::Message*> repaetedMsg = decoder.getMessageArray(out.num());
                if (repaetedMsg.empty()) return;
                for (uint32_t idx = 0; idx < repaetedMsg.size(); ++idx) {
                    decoder.setCurMsg(repaetedMsg.at(idx));
                    typename TypeTraits<T>::Type item = typename TypeTraits<T>::Type();
                    decoder.operator>>(item);
                    out.value().push_back(item);
                }
                decoder.setCurMsg(msg);
            }
        };
        template <>
        struct valueDecoder<0> {
            template <typename OUT>
            static void decode(serializePair<OUT>& out, PBDecoder& decoder) { decoder.decodeValue(out); }

            template <typename OUT>
            static void decodeRepaeted(serializePair<OUT>& out, PBDecoder& decoder) {
                decoder.decodeRepaeted(out);
            }
        };
        friend struct valueDecoder<0>;
        friend struct valueDecoder<1>;

        PBDecoder(const PBDecoder&);
        PBDecoder& operator=(const PBDecoder&);
    public:
        PBDecoder(const uint8_t* sz, uint32_t size);
        ~PBDecoder();

        template<typename T>
        bool operator>>(T& value) {
            if (_bParseRet)
                serializeWrapper(*this, value);
            return _bParseRet;
        }

        template<typename T>
        PBDecoder& operator&(serializePair<T> value) {
            valueDecoder<isMessage<T>::YES>::decode(value, *this);
            return *this;
        }

        template<typename T>
        PBDecoder& operator&(serializePair<std::vector<T> > value) {
            valueDecoder<isMessage<T>::YES>::decodeRepaeted(value, *this);
            return *this;
        }
    private:
        proto::Message* getMessage(int32_t number);
        std::vector<proto::Message*> getMessageArray(int32_t number);

        template<typename T>
        void decodeValue(serializePair<T>& v);

        template<typename T>
        void decodeRepaeted(serializePair<std::vector<T> >& v);

        proto::Message* getCurMsg() { return _curMsg; }
        void setCurMsg(proto::Message* msg) { _curMsg = msg; }
    };

}

#endif