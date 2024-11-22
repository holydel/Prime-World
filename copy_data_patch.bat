SETLOCAL ENABLEDELAYEDEXPANSION
@FOR /F "delims=pw/branches/r1117/ tokens=*" %%i IN ('git diff-tree --no-commit-id --name-only -r %1') DO @(
@SET F=%%i
@SET F=!F:/=\!
@ECHO !F!
@SET FILEDEST=Data_Patch\!F!
@SET DIRDEST=!FILEDEST:~0,-1!
@IF NOT EXIST "!DIRDEST!" (
	@mkdir "!DIRDEST!"
)
@copy /Y "pw\branches\r1117\!F!" "!FILEDEST!"
)
@ENDLOCAL