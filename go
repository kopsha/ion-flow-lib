#!/usr/bin/env bash
set -euo pipefail

CONTEXT=${CONTEXT:-local}
VERSION_TAG=$(git describe || git branch --show-current)
VERSION=${VERSION:-$VERSION_TAG}


build()
{
    mkdir -p build
    cmake -S . -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build ./build -- -j$(nproc)
}

cover_build()
{
    mkdir -p build/coverage
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON
}


main()
{
    case $1 in
        build)
            build
            ;;
        clean)
            printf "Wiping ./build folder completely.\n"
            rm -rf ./build
            ;;
        selfcheck)
            build
            ctest -T Test --test-dir ./build
            ;;
        cover)
            cover_build
            ctest -T Test --test-dir ./build
            cmake --build ./build --target coverage
            ;;
        tdd)
            build
            find ./src ./tests ./examples -name "*.cpp" -o -name "*.h" | entr -rc sh -c "cmake --build ./build && ctest --output-on-failure --test-dir ./build/tests"
            ;;
        *)
            "$@"
            ;;
    esac
}

printf "\t..: Running version $VERSION on $CONTEXT\n"
main "$@"

