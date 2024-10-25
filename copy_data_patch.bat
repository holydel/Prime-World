SETLOCAL ENABLEDELAYEDEXPANSION
@FOR /F "delims=pw/branches/r1117/ tokens=*" %%i IN ('git show %1 --name-only') DO @(
@SET F=%%i
@SET F=!F:/=\!
@ECHO !F!
xcopy /Y /I "pw\branches\r1117\!F!" "Data_Patch\!F!\"
)
@ENDLOCAL