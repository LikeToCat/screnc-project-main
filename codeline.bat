@echo off
REM 修改 cloc 可执行文件名或路径（如果在当前目录，保留 .\cloc-2.06.exe）
set CLOC_EXE=.\cloc-2.06.exe
set TARGET=E:\MyOwnProgram\Win7ScreenRecProject\ScrnrecMain
set EXCLUDE=third_library
set OUT=E:\MyOwnProgram\cloc_report.json

"%CLOC_EXE%" "%TARGET%" --exclude-dir=%EXCLUDE% --json --out="%OUT%"
if %ERRORLEVEL% NEQ 0 (
  echo cloc 返回非零退出码。请检查上面的输出。
) else (
  echo cloc 完成。报告保存到 %OUT%
)
pause