# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/BME/grafika/hf2/src/freeglut"
  "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix/src/glut-build"
  "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix"
  "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix/tmp"
  "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix/src/glut-stamp"
  "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix/src"
  "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix/src/glut-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix/src/glut-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/BME/grafika/hf2/cmake-build-debug-mingw/glut-prefix/src/glut-stamp${cfgdir}") # cfgdir has leading slash
endif()
