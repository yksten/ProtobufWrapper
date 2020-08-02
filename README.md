# ProtobufWrapper
一个C++版轻量级的Protobuf解析库，实现了C++结构体和Google Protobuf的互转。可通过proto文件生成C++原生结构体文件（支持std::string、std::vector和std::map）。

# 为什么要为Protobuf重复造轮子？
Goolge的原生C++解析库功能又太强大，代码膨胀较大，像反射、定义service很多时候都用不到。目前第三方的解析库都是C语言版的，序列化时需要手动分配内存，使用成本偏高。

# 如何使用
编写proto文件，使用tool文件夹中的代码生成的protoc可执行文件，命令行执行./protoc --struct_out=./ test.proto，即可生成相应的C++文件。
```
struExamples items;
// items赋值 ...
// 序列化
serialization::BufferWrapper buffer;
serialization::PBEncoder encoder(buffer);
encoder << items;
// 反序列化
struExamples items2;
serialization::PBDecoder decoder(buffer.data(), buffer.size());
decoder >> items2;
```
# 后续计划
目前支持int32、int64、uint32、uint64、sint32、sin64、bool、enum、fixed32、fixed64、float、double、string、bytes、embedded messages、packed repeated fields和map（proto3）。不支持enum，建议使用int32代替。sfixed32、sfixed64留有接口，暂未实现转换。暂未实现has功能。
后续代码考虑支持has功能，并和struct2x融合，支持struct和json、struct和protobuf的转换。
代码没有进行全量测试验证，转换可能会有问题。
