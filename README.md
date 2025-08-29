# Webview2Test-tcc

Use C language and tcc compiler to pack front-end project to executable program(64 bit)


## How to compile

Used compiler: **Tiny C Compiler** (tcc version 0.9.27)

Run `build.bat` or following command:

```powershell
windres res/res.rc -O coff res/res.res
tcc -impdef publish/Webview2Loader.dll -o Webview2Loader.def

tcc main.c MyHttpServer.c res/res.res -o publish/Herta.exe -Iwinapiwv2 -lws2_32 -lcomctl32 Webview2Loader.def -Wl,-subsystem=gui -Dinterface=struct -DDEFINE_ENUM_FLAG_OPERATORS(arg)= -D_COM_Outptr_=
```


## Dependencies

### Include

`winapiwv2/`: copied winapi according [winapiext-tcc](https://github.com/Lasereyes5/winapiext-tcc)

### Icon in res/

`icon/`: Original files of `WebView2.ico`

`icon/WebView2.ico`: 16x16 pixel redraw of EdgeWebView2.ico

`icon/EdgeWebView2.ico`: [EdgeWebView2.ico](https://github.com/MicrosoftEdge/WebView2Samples/blob/main/media%2Ficon%2FEdgeWebView2.ico) in [WebView2Samples](https://github.com/MicrosoftEdge/WebView2Samples/blob/main/media%2Ficon%2FEdgeWebView2.ico)/media/

`icon/WebView2.tsp`: Original [Hornil StylePix](https://www.hornil.com/en/stylepix/download/) Project of WebView2.ico
