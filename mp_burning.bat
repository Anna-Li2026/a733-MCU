@echo ================================================================================================================
@echo 烧录前配置：
@echo 1.安装segger软件
@echo 2.安装路径C:\Program Files (x86)\SEGGER\JLink_V490\JFlash.exe复制到JFLASH，如下所示
@echo ================================================================================================================
@echo 生产烧录：
@echo 1.双击mp_burning.bat
@echo 2.查看烧录结果
@echo ================================================================================================================
@echo off

::设置工具地址
set JFLASH="C:\Program Files (x86)\SEGGER\JLink_V490\JFlash.exe"
set GEN_GUID=TOOL\GuidGen.exe
set GEN_MAC=TOOL\gen_mac.exe


::以下禁止改动
@echo 当前生产路径:%~dp0
::产生唯一GUID
set tmp_file=_tmp_file_%RANDOM%.txt
%GEN_GUID% --format=HexH --count=1 > %tmp_file%
set /p guid=<%tmp_file%
del %tmp_file%
echo guid=%guid%

::根据GUID生成MAC地址
%GEN_MAC% wiznet_tmp.bin ..\wiznet.bin 0x0000000f0 %guid%

::使用JLINK烧录
%JFLASH% -openprjTOOL\TL_LPDDR4_STM32_W5500_MP.jflash -connect -openTOOL\wiznet_tmp.bin,0x08000000 -auto -exit
if %errorlevel% equ 0 (
	echo 烧录TL_LPDDR4_STM32_W5500_MP固件包成功
) else (
	echo 烧录固件失败 
)

pause
