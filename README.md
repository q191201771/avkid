# avkid

用ffmpeg做一些音视频相关的工作~

## 现在可以做啥

### 1. http-flv截图

```
/src/exe_rtmp2jpeg.cc
input: 拉取http-flv格式流
output: 把视频中的每个I帧解码再编码成jpeg图片文件存放在本地磁盘
```

### 2. http-flv录制

```
/src/exe_rtmp2flv.cc
input: 拉取http-flv格式流
output: 解码再编码（编码格式和解码格式相同）后存储为本地flv格式文件
```

## 我的环境

ffmpeg version 3.4.2

Apple LLVM version 8.0.0 (clang-800.0.42.1)

Darwin xxx.local 15.6.0 Darwin Kernel Version 15.6.0: Thu Jun 23 18:25:34 PDT 2016; root:xnu-3248.60.10~1/RELEASE_X86_64 x86_64

