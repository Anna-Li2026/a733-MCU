@echo off
rem 从git获取当前代码的commit和tag并输出到文件version
git rev-parse --short HEAD > version
set /p commit=<./version
 git describe --tags --dirty=d --abbrev=0 > version
set /p tag=<./version

rem 输出到文件
echo #define __TAG__ "%tag%" > version
echo #define __COMMIT__ "%commit%" >> version
echo #define __BUILT__ "%date:~0,10% %time:~0,8%" >> version
