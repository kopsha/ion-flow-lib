#!/usr/bin/env bash
set -euo pipefail

CONTEXT=${CONTEXT:-local}
VERSION_TAG=$(git describe || git branch --show-current)
export VERSION=${VERSION:-$VERSION_TAG}
export CMAKE_BUILD_TYPE="Debug";

BUILD_PARAMS="
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
"
TEST_PARAMS="
-DBUILD_TESTS=ON
"

build()
{
    extra_args=$@
    cmake ${BUILD_PARAMS} ${extra_args} -S . -B ./build
    cmake --build ./build -- -j$(nproc)
}

build_dist()
{
    mkdir -p build/dist
    rm -rf build/dist/*
    extra_args=$@
    cmake ${BUILD_PARAMS} ${extra_args} -S . -B ./build
    cmake --build ./build -- -j$(nproc)
    cmake --install ./build --prefix build/dist
}

main()
{
    mkdir -p build
    goal=${1:-}; shift
    case ${goal} in
        debug|release)
            export CMAKE_BUILD_TYPE=${goal^};
            EXTRA_CMAKE_ARGS=$@
            build_dist $EXTRA_CMAKE_ARGS
            ;;
        clean)
            printf "Wiping ./build folder completely.\n"
            rm -rf ./build
            ;;
        check)
            build ${TEST_PARAMS}
            ctest --extra-verbose --output-on-failure --test-dir ./build/tests
            ;;
        run)
            build "-DBUILD_EXAMPLES=ON"
            ./build/examples/ionExample
            ;;
        tdd)
            build
            find ./src ./tests ./examples -name "*.cpp" -o -name "*.h" | entr -rc bash -c "./go check"
            ;;
        *)
            "$@"
            ;;
    esac
}

printf "\t..: Using version $VERSION on $CONTEXT\n"
main "$@"

