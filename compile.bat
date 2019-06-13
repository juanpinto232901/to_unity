D:
cd build
#cmake ..\ -G "Visual Studio 14" -DPROJECT_SOURCE_DIR=..\ -DPROJECT_BINARY_DIR=..\Build -DQT_LIBRARIES=C:\Qt\4.8.6\lib -DQT_INCLUDE_DIR=C:\Qt\4.8.6\include -DGLEW_INCLUDE_DIR=C:\glew-2.1.0\include -DGLEW_LIBRARY=C:\glew-2.1.0\lib\Release\Win32\glew32.lib -DGLUT_LIBRARY=C:\glut_win
#cmake ..\ -G "Visual Studio 12" -DPROJECT_SOURCE_DIR=C:\GLSL-Debugger-vs2015 -DPROJECT_BINARY_DIR=C:\GLSL-Debugger-vs2015\Build -DQT_LIBRARIES=C:\Qt\4.8.6\lib -DQT_INCLUDE_DIR=C:\Qt\4.8.6\include -DGLEW_INCLUDE_DIR=C:/glew-2.1.0/include -DGLEW_LIBRARY=C:/glew-2.1.0/lib/Release/Win32/glew32.lib -DGLUT_LIBRARY=C:\glut_win -DUSE_MESA=ON
#cmake ..\ -G "Visual Studio 14" -A x64 -DPROJECT_SOURCE_DIR=C:\GLSL-Debugger-vs2015 -DPROJECT_BINARY_DIR=C:\GLSL-Debugger-vs2015\Build -DQT_LIBRARIES=C:\Qt\4.8.6\lib -DQT_INCLUDE_DIR=C:\Qt\4.8.6\include -DGLEW_INCLUDE_DIR=C:\glew-2.1.0\include -DGLEW_LIBRARY=C:\glew-2.1.0\lib\Release\Win32\glew32.lib -DGLUT_LIBRARY=C:\glut_win -DUSE_MESA=ON
#cmake ..\ -G "Visual Studio 15" -A x64 -DBOOST_ROOT:PATH=D:\boost_1_68_0 -DBOOST_INCLUDEDIR:PATH=D:\boost_1_68_0\boost -DBOOST_LIBRARYDIR:PATH=D:\boost_1_68_0\lib64-msvc-14.1 -DLIBJPEGTURBO_ROOT_DIR=D:\libjpeg-turbo64 -DLIBJPEGTURBO_INCLUDE_DIR=D:\libjpeg-turbo64\include -DLIBJPEGTURBO_JPEG_LIBRARY=D:\libjpeg-turbo64\lib
cmake ..\ -G "Visual Studio 15" -A x64
cd ..


