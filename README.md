UEFI-OSLoader
=============

## Overview
UEFI環境で動くシンプルなOS/UEFIアプリ・ローダーです。
[EDKII](https://github.com/tianocore/edk2)を使って作成。
UEFIの勉強を兼ねた最低限の実装です。
コードの理解しやすさ重視で作っています。

## Feature
* Simple
* Easy to understand

## Build
Environment: Ubuntu 14.10 x86_64, gcc 4.9.1
Target: x64
- apt-get install git make gcc uuid-dev g++ python nasm
- git clone https://github.com/fgken/uefi-osloader.git
- cd uefi-osloader
- make -C BaseTools
- . edksetup.sh
- vim Conf/target.txt
	- 次のように3箇所変更
	- "ACTIVE_PLATFORM = Nt32Pkg/Nt32Pkg.dsc" => "ACTIVE_PLATFORM = AppPkg/AppPkg.dsc"
	- "TARGET_ARCH = IA32" => "TARGET_ARCH = X64"
	- "TOOL_CHAIN_TAG = MYTOOLS" => "TOOL_CHAIN_TAG = GCC49"
		- gccのバージョンが4.9.1だったのでGCC49
- build

これでビルド完了。
OSローダーはAppPkgのHelloに実装しているので、
ソースコードはuefi-osloader/AppPkg/Applications/Helloに、
バイナリはuefi-osloader/Build/AppPkg/DEBUG_GCC49/X64/にあります。

