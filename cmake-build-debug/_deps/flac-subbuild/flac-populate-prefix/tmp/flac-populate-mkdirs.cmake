# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-src"
  "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-build"
  "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-subbuild/flac-populate-prefix"
  "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-subbuild/flac-populate-prefix/tmp"
  "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-subbuild/flac-populate-prefix/src/flac-populate-stamp"
  "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-subbuild/flac-populate-prefix/src"
  "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-subbuild/flac-populate-prefix/src/flac-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-subbuild/flac-populate-prefix/src/flac-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/pliki/studia/semestr 4/Rozporszone/Projekt/Shipers/cmake-build-debug/_deps/flac-subbuild/flac-populate-prefix/src/flac-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
