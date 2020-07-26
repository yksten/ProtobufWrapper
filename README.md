# ProtobufWrapper
一个C++版轻量级的Protobuf解析库，实现了C++结构体和Google Protobuf的互转。可通过proto文件导出C++文件。

# 为什么要为Protobuf重复造轮子？
目前第三方的解析库都是C语言版的，使用成本偏高。Goolge的原生解析库功能太丰富，代码膨胀较大。

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
暂不支持map，后续支持。代码也会和struct2x融合，支持struct和json、struct和protobuf的转换。
该代码暂未大量测试验证，转换可能会有问题。
