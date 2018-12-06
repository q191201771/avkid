# avkid

基于ffmpeg的小型框架，目前聚焦于直播相关的业务。

### 特性

* api尽量简单，隐藏细节
* 不过度封装
* module化
* 所有对象生命周期自动化管理
* av数据管理，内存优化，无多余拷贝，零泄漏
* 所有module支持同步/异步，只需配置一个参数，其他无差别
* 支持设置时长
* 音视频单独控制
* 支持超时

### avkid的源码文件结构图

![image](https://note.youdao.com/src/516BB5D909B64E3EBED8B337ABF9A195)

### 流媒体数据

流媒体的数据可以从内容和格式两个维度进行分类

内容：

* Audio 音频
* Video 视频

格式：

- original 原始未编码（或已解码）
- encoded 已经编码

一切算法（或者说策略、操作、逻辑）都是服务于数据。ffmpeg中与音视频数据最相关的两个数据结构：

* AVFrame 对应original
* AVPacket 对应encoded

TODO 补充，说明封装格式，以及mux，demux

### module

avkid中各module的输入输出

![image](http://note.youdao.com/yws/res/18574/B429C435E40D46019569D5F1D5FD856E)

使用：module与module之间，只要输入输出的格式匹配，就可以挂载（avkid中的combine概念），业务方也可以实现自己的module进行挂载

#### 构建应用

1. 录制

不需要编解码

![image](http://note.youdao.com/yws/res/18578/596E9C213EFE4285AF446E506BC0547E)

2. 截图

![image](http://note.youdao.com/yws/res/18580/0C82E3F4331348449D598D0F8E7FCE39)

3. 将视频转化成黑白，上下或左右翻转（还没尝试的打水印、高斯处理等等）

利用了filter模块。ffmpeg提供了非常多的filter可供使用，我们只需要传入对应的字符串命令即可。

基于avkid模块化可插拔的特性，我们可以在任意环节对AVFrame做filter操作。比如录制黑白视频或截黑白的图片。

![image](http://note.youdao.com/yws/res/18597/E5424B45CA044B4DAC9F185BEE968436)

4. 合流

演示如何更自由的组合avkid module，以及插入业务方的处理

![image](http://note.youdao.com/yws/res/18602/A68F9CE4E56E41F7B7DD7631CC47CB88)

5. avkid和其他库、框架协助

比如其他网络库协议栈收到的数据，只需要将数据适配转换成AVPacket格式，就可以挂载decode module

