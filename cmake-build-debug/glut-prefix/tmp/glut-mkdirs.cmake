# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/src/freeglut"
  "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix/src/glut-build"
  "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix"
  "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix/tmp"
  "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix/src/glut-stamp"
  "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix/src"
  "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix/src/glut-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix/src/glut-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/pacsi/Documents/Projects/School/5. felev/Grafika/Hf1/cmake-build-debug/glut-prefix/src/glut-stamp${cfgdir}") # cfgdir has leading slash
endif()
