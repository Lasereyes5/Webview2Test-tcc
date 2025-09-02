# Webview2Test-tcc

使用C语言将前端项目打包为exe程序(64 bit)

该项目与[diyKards](https://www.bilibili.com/read/cv42630769 "diyKards安卓APP")项目无关，仅为作者个人为diyKards网页资源修改Webview2Test-tcc作为Windows端的WebView2套壳程序方便使用diyKards网页资源。

## 用法

将发行版所有文件复制到Windows端打包的web-diyKards目录下打开程序，或者解压安卓端安装包的`assets/public`路径下的所有资源复制到发行版目录的dist文件夹中打开程序。

### 使用不便的地方

- 若卡牌图片为空白或数值不显示可能需要点击数值等面板来刷新
- 若卡牌数值字体有问题可能需要点击+-来刷新
- 会生成Webview2.Data缓存文件夹（可删除，但会清空模板等个人数据）
- 若无法保存图片，可能需要右键卡图手动保存到本地

## 如何编译本项目

使用的编译器为 **Tiny C Compiler** (tcc version 0.9.27, GNU windres 2.41)

运行`build.bat`或以下命令：

```powershell
windres res/res.rc -O coff res/res.res
tcc -impdef publish/Webview2Loader.dll -o Webview2Loader.def

tcc main.c MyHttpServer.c res/res.res -o publish/Diy-Kards.exe -Iwinapiwv2 -lws2_32 -lcomctl32 Webview2Loader.def -Wl,-subsystem=gui -Dinterface=struct -DDEFINE_ENUM_FLAG_OPERATORS(arg)= -D_COM_Outptr_=
```


## 依赖项

### 头文件

`winapiwv2/`: 参照[winapiext-tcc](https://github.com/Lasereyes5/winapiext-tcc)的方法复制自winapi

### res目录里的图标

`icon/fulan.ico`: 复制自diyKards网页资源的图标
