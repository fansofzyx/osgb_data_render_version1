@echo off

call :prepare_env
call :build_vs

goto :EOF

:prepare_env

echo "call env.bat if exist"
if exist env.bat (call env.bat)

goto :EOF

:build_vs

if defined VC_DIR  (
	if defined QT5_10 (echo "VC_DIR and QT5_10 are set.") else (
		echo "please set the 2 env variables: VC_DIR, QT5_10 in env.bat, and retry again."
		pause
		exit
	)
)  else  (
	echo "please set the 2 env variables: VC_DIR, QT5_10 in env.bat, and retry again."
	pause
	exit
)

SET PATH=%QT5_10%;%PATH%
call "%VC_DIR%\vcvarsall.bat" amd64

qmake -r -tp vc ./osgbTest.pro
start devenv.exe osgbTest.sln

goto :EOF