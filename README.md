## Arcsoft SDK4.1 C++ for Linux Demo

### 0 前言

​	人脸识别技术做作为图像处理领域中的分支之一，已经相当成熟，目前有很多人脸技术的SDK包，包括虹软，百度等开发包，本demo是基于虹软 SDK4.1 c++ for linux 的一个人脸识别demo，至于选择虹软的理由，当然是因为其简单易用，最重要的是 免费， 目前官网提供的免费sdk 支持linux64，window64，window32，以及ios的3.0 及以前版本，并提供了相关的开发文档，开发起来很是方便。见虹软官网[开发者中心 (arcsoft.com.cn)](https://ai.arcsoft.com.cn/ucenter/resource/build/index.html#/addFreesdk/1002)

本文是针对sdk4.1 c++for linux 的一个demo，使用后的体验给我的感受是：sdk4.1的相比以前的版本，是被速度更快，更稳定，实时性能更好，同时，也兼容了口罩识别，可以根据传入的参数选择想要是识别效果。

#### 注意

>SDK4.1的激活码是和硬件绑定的，一个激活码只能绑定一个硬件设别，如果在别的设备激活过，则要准备**新的激活码**！

```
代码中给出的 SDK4.1 用户-密码-激活码  只是样例，并不是真实能用的，若想使用，需要取虹软光网申请。
申请方式见下网址：
https://blog.csdn.net/weixin_41231810/article/details/120156439
```



### 1. 编译环境 ：

	* QT5.15以上
	* arcsoftSDK4.1  
	* 在识别过程中会出现**81927**的人脸检测错误码，这是正常现象，可能原因包括识别距离远，或者获取的帧脸部数据不清楚，，若不想出现此提示，找到**videoft.cpp**文件的中相关代码注释掉即可。

### 2. 使用前准备

 * 将lib目录下的所有.so文件复制到 /usr/local/lib 目录下 ，再更新动态链接库

 * ```shell
   # arch linux下 需要安装qt5-multimedia包
   sudo pacman -S qt5-multimedia
   # ubuntu 下 需要安装 qtmultimedia5-dev 包
   sudo apt-get install qtmultimedia5-dev
   # 复制相关动态链接库
   sudo cp libarcsoft_face_engine.so /usr/local/lib/
   sudo cp libarcsoft_face.so /usr/local/lib/
   # 跟新链接库
   sudo ldconfig
   # 如果还是无法运行
   # 将项目下的 .pro 文件内的LIBS对应的内容 去掉“lib/”，再进行编译
   # 即： lib/libarcsoft_face.so -> libarcsoft_face.so
   #  lib/libarcsoft_face_engine.so->libarcsoft_face_engine.so
   
   # 其他问题可以在终端执行： ./ASF_fr
   # 根据 编译器提示进行处理，或者安装相关的 库文件 包文件
   ```

### 3. 运行

* 程序已经编译好，在程序文件的根目录下打开终端，执行以下名利你个,便可运行程序，注意检测摄像头是否连接。

  ```shell
  ./ASF_fr   
  ```




### 4. 重新构建运行

>在上述步骤2之后，点解qt菜单栏中的构建-> 清理项目 （或者直接重新构建项目）->运行



