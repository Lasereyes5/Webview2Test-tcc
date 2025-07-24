# Webview2Test
使用C语言将前端项目打包为exe程序

## 如何编译本项目
使用的编译器为 **MinGW** (gcc version 14.2.0)
```powershell
windres res/res.rc res/res.o
gcc main.c MyHttpServer.c res/res.o -o publish/Herta.exe -lws2_32 -lpublish/Webview2Loader -lcomctl32 -L. -w -mwindows
```
