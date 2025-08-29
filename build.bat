@echo off
title Lain

windres res/res.rc -O coff res/res.res
tcc -impdef publish/Webview2Loader.dll -o Webview2Loader.def

set cc=tcc
set files=main.c MyHttpServer.c res/res.res -o publish/Herta.exe
set includes=-Iwinapiwv2
set libs=-lws2_32 -lcomctl32 Webview2Loader.def
REM Defines from gcc preprocess result
set defines=-Dinterface=struct -DDEFINE_ENUM_FLAG_OPERATORS(arg)= -D_COM_Outptr_=
set args=%includes% %libs% %defines% %files% -Wl,-subsystem=gui

%cc% %args%
