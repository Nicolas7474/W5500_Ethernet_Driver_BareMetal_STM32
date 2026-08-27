@echo off
:: Force the script to always run from the folder where this batch file lives (project root)
cd /d "%~dp0"

echo ====================================
echo Running Ninja Build...
echo ====================================

:: Temporarily add the STM32 toolchain bin directory to the PATH for compilation and size utilities
set "PATH=C:\Users\rakma\AppData\Local\stm32cube\bundles\gnu-tools-for-stm32\14.3.1+st.2\bin;%PATH%"

"C:\Espressif\tools\ninja\1.12.1\ninja.exe" -C Debug

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build failed! Flash cancelled.
    exit /b %errorlevel%
)

echo.
echo ====================================
echo Flashing Target via OpenOCD...
echo ====================================

"C:\Espressif\tools\openocd-esp32\v0.12.0-esp32-20260424\openocd-esp32\bin\openocd.exe" -s "C:\Espressif\tools\openocd-esp32\v0.12.0-esp32-20260424\openocd-esp32\share\openocd\scripts" -f interface/stlink.cfg -c "transport select swd" -f target/stm32f4x.cfg -c "adapter speed 1000" -c "stm32f4x.cpu configure -event reset-start { adapter speed 1000 }" -c "program C:/Users/rakma/Documents/STM32CubeIDE/workspace_1.19.0/W5500_Driver_BareMetal_STM32/Debug/W5500_Driver_BareMetal_STM32.elf verify reset exit"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Flashing failed!
    exit /b %errorlevel%
)

echo.
echo [SUCCESS] Build and Flash completed successfully!