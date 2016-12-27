# visualassistant

# OpenCV

## macOS

```
brew tap homebrew/science
brew install opencv3
```

## win
```
cmake -G "MinGW Makefiles" -D CMAKE_CXX_COMPILER="C:/Qt/Tools/mingw530_32/bin/g++.exe" -D CMAKE_C_COMPILER="C:/Qt/Tools/mingw530_32/bin/gcc.exe" -D WITH_IPP=OFF MAKE_MAKE_PROGRAM=mingw32-make.exe ..\sources
mingw32-make
mingw32-make install
```
