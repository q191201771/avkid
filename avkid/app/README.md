## app_mp4_2_jpeg

对http mp4截图

### 使用说明

Usage: app_mp4_2_jpeg <mp4 url> <output dir> <width:set 0 to copy> <height:set 0 to copy> <interval ms>

#### 参数说明

* mp4 url: http mp4地址
* output dir: 图片输出目录
* width: 生成图片的宽，如果为0，则使用mp4的宽
* height: 生成图片的高，如果为0，则使用mp4的高
* interval ms: 截图时间间隔，单位毫秒

#### 程序返回值

在stderr最后追加一行int类型程序返回值。

如果一切正常，返回截图总张数，
如果失败，则返回负数错误码，错误码含义

* -1 程序的输入参数有误
* -2 图片输出目录有误或无法创建
* -3 打开http mp4失败，比如文件不存在
* -4 读取http mp4时失败，比如中途网络中断
* -5 mp4解码失败

#### 其他

图片文件名为<index>.jpeg ，index为累加的计数，初始值为1

程序内部有重试机制，当发生错误时，会间隔2秒重试，总共重试3次

如果只是生成部分截图没有生成所有截图，那么程序返回值为失败错误码，而非已截图数量（但目录下可能有部分截图）
