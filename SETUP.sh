#!/bin/sh

script_failed() {
  echo "SETUP.sh $1: \033[0;31mSCRIPT FAILED\033[0m";
  exit 1;
}

option="${1}"
case ${option} in
  all)
    ninja build/TestCatch2;
    ./build/TestCatch2 || script_failed

    ninja build/libMTBase64.so build/MTBase64.a || script_failed

    cp MTBase64/MTBase64.hpp build/CPP_Headers/MTBase64.hpp
    cp MTBase64/MTBase64.tcc build/CPP_Headers/MTBase64.tcc

    rm build/TestCatch2 2> /dev/null
    rm build/MTBase64.o 2> /dev/null
    rm build/MTBase64.so.o 2> /dev/null

    echo "SETUP.sh $1: \033[0;32mSCRIPT SUCCESS\033[0m";
    echo "\033[1;33mbuild/CPP_Headers/MTBase64.hpp: Main header file\033[0m"
    echo "\033[1;33mbuild/CPP_Headers/MTBase64.tcc: Template implementation file\033[0m"
    echo "\033[1;33mbuild/libMTBase64.so: Shared library file\033[0m"
    echo "\033[1;33mbuild/MTBase64.a: Static library file\033[0m"

    exit 0
    ;;
  clean)
    ninja -t clean
    rm -R build/CPP_Headers/* 2> /dev/null
    echo "Cleaning... build/CPP_Headers"

    exit 0
    ;;
  static)
    ninja build/TestCatch2;
    ./build/TestCatch2 || script_failed

    ninja build/MTBase64.a || script_failed

    cp MTBase64/MTBase64.hpp build/CPP_Headers/MTBase64.hpp
    cp MTBase64/MTBase64.tcc build/CPP_Headers/MTBase64.tcc

    rm build/TestCatch2 2> /dev/null
    rm build/MTBase64.o 2> /dev/null

    echo "SETUP.sh $1: \033[0;32mSCRIPT SUCCESS\033[0m";
    echo "\033[1;33mbuild/CPP_Headers/MTBase64.hpp: Main header file\033[0m"
    echo "\033[1;33mbuild/CPP_Headers/MTBase64.tcc: Template implementation file\033[0m"
    echo "\033[1;33mbuild/MTBase64.a: Static library file\033[0m"

    exit 0
    ;;
  shared)
    ninja build/TestCatch2;
    ./build/TestCatch2 || script_failed

    ninja build/libMTBase64.so || script_failed

    cp MTBase64/MTBase64.hpp build/CPP_Headers/MTBase64.hpp
    cp MTBase64/MTBase64.tcc build/CPP_Headers/MTBase64.tcc

    rm build/TestCatch2 2> /dev/null
    rm build/MTBase64.so.o 2> /dev/null

    echo "SETUP.sh $1: \033[0;32mSCRIPT SUCCESS\033[0m";
    echo "\033[1;33mbuild/CPP_Headers/MTBase64.hpp: Main header file\033[0m"
    echo "\033[1;33mbuild/CPP_Headers/MTBase64.tcc: Template implementation file\033[0m"
    echo "\033[1;33mbuild/libMTBase64.so: Shared library file\033[0m"

    exit 0
    ;;
  help)
    echo "SETUP.sh: List of all arguments:\n"
    echo "\tall: Build both the static and shared library"
    echo "\tstatic: Build only the static library file"
    echo "\tshared: Build only the shared library file"
    echo "\tclean: Remove all build files"
    echo "\thelp: Show this list"
    ;;
  *)
    echo "$0: Wrong usage. Type 'SETUP.sh help' for more information."
    exit 1
    ;;
esac
