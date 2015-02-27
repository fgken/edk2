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

1. apt-get install git make gcc uuid-dev g++ python nasm
1. git clone https://github.com/fgken/uefi-osloader.git
1. cd uefi-osloader
1. make -C BaseTools
1. . edksetup.sh
1. vim Conf/target.txt
	- 次のように3箇所変更
	- "ACTIVE_PLATFORM = Nt32Pkg/Nt32Pkg.dsc" => "ACTIVE_PLATFORM = AppPkg/AppPkg.dsc"
	- "TARGET_ARCH = IA32" => "TARGET_ARCH = X64"
	- "TOOL_CHAIN_TAG = MYTOOLS" => "TOOL_CHAIN_TAG = GCC49"
		- gccのバージョンが4.9.1だったのでGCC49
1. build

これでビルド完了。
OSローダーはAppPkgのHelloに実装しているので、
ソースコードはuefi-osloader/AppPkg/Applications/Helloに、
バイナリはuefi-osloader/Build/AppPkg/DEBUG_GCC49/X64/にあります。

## Running

```shell
cd uefi-osloader/AppPkg/Applications/Hello
. make.sh
cd ../../../
mkdir running-dir
cd running-dir
wget http://sourceforge.net/projects/edk2/files/OVMF/OVMF-X64-r15214.zip
unzip OVMF-X64-r15214.zip OVMF.fd
rm OVMF-X64-r15214.zip
mkdir -p image/EFI/BOOT
cp ../Build/AppPkg/DEBUG_GCC48/X64/Hello.efi image/
cp ../AppPkg/Applications/Hello/out-serial-A.elf image/output.elf
qemu-system-x86_64 -nographic -bios OVMF.fd -hda fat:image -net none
```

in qemu console

```shell
UEFI Interactive Shell v2.0
EDK II
UEFI v2.40 (EDK II, 0x00010000)
Mapping table
      FS0: Alias(s):HD7a1:;BLK3:
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)
     BLK2: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
     BLK4: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
     BLK0: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x0)/Floppy(0x0)
     BLK1: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x0)/Floppy(0x1)
Press ESC in 5 seconds to skip startup.nsh or any other key to continue.
Shell> fs0:
FS0:\> ls
Directory of: FS0:\
02/27/2015  18:36              31,712  Hello.efi
02/27/2015  18:33 <DIR>         8,192  EFI
02/27/2015  18:45               1,460  output.elf
02/27/2015  09:49               1,403  NvVars
          3 File(s)      34,575 bytes
          1 Dir(s)
FS0:\> Hello output.elf
Load the file = output.elf
Load the program section of the ELF executable into memory
Execute the program
A
```

"C-a x" : exit from qemu


