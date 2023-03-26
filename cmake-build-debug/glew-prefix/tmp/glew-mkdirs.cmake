# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/BME/grafika/hf1/src/glew"
  "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix/src/glew-build"
  "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix"
  "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix/tmp"
  "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix/src/glew-stamp"
  "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix/src"
  "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix/src/glew-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix/src/glew-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/BME/grafika/hf1/cmake-build-debug/glew-prefix/src/glew-stamp${cfgdir}") # cfgdir has leading slash
endif()
