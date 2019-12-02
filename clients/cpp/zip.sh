#!/bin/bash
zip -r CodeSide.zip utils/ arena/ model/   -i \*.h \*.cpp \*.hpp --exclude=cmake-build-debug/*
zip -ur CodeSide.zip  MyStrategy.cpp MyStrategy.hpp CMakeLists.txt Debug.cpp Debug.hpp TcpStream.cpp TcpStream.hpp main.cpp Stream.cpp Stream.hpp 
