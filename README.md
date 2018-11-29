# avkid

学习ffmpeg，做一些音视频方便的事，后面会写文档介绍avkid中各模块的功能划分以及如何组合工作~

## 核心模块

TODO

## 使用avkid的示范

### 1. http-flv截图

```
/src/exe_rtmp2jpeg.cc
input: http-flv | flv文件（其他格式待测试）
output: 对视频中的I帧存储为本地jpeg图片文件

* 支持截图n张后停止
* 支持缩放（指定截图后宽高）
```

### 2. http-flv录制

```
/src/exe_rtmpdump.cc
input: http-flv | flv文件（其他格式待测试）
output: 存储为本地flv格式文件（其他格式待测试）

* 支持录制固定时长后停止

TODO
* 支持只录制音频或视频
```

### 3. http-flv解码再编码后录制

```
/src/exe_rtmp_decode_encode.cc
```

## 未使用avkid的程序（和音视频相关，临时先放这）

### 1. 分析flv文件中的nalu包

```
/src/exe_tag_nalus.cc
分析flv文件中的video tag中的nalu包的情况
```

## 我的环境

ffmpeg version 3.4.2

Apple LLVM version 8.0.0 (clang-800.0.42.1)

Darwin xxx.local 15.6.0 Darwin Kernel Version 15.6.0: Thu Jun 23 18:25:34 PDT 2016; root:xnu-3248.60.10~1/RELEASE_X86_64 x86_64

