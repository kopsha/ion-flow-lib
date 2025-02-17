#!/usr/bin/env bash
set -euo pipefail

CONTEXT=${CONTEXT:-local}
VERSION_TAG=$(git describe || git branch --show-current)
export VERSION=${VERSION:-$VERSION_TAG}
export CMAKE_BUILD_TYPE="Debug";

BUILD_PARAMS="
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
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
            printf "Wiping clean the ./build folder.\n"
            find ./build -mindepth 1 -maxdepth 1 ! -name '_deps' -exec rm -rf {} +
            ;;
        check)
            build -DBUILD_TESTS=ON
            ctest --verbose --output-on-failure --test-dir ./build/tests
            ;;
        cover)
            mkdir -p build/coverage
            build -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
            ctest --output-on-failure --test-dir ./build/tests
            cmake --build ./build --target coverage
            ;;
        run)
            build "-DBUILD_EXAMPLES=ON"
            ./build/examples/ionExample
            ;;
        tdd)
            build
            find ./src ./tests ./examples -name "*.cpp" -o -name "*.h" | entr -rc bash -c "./go check"
            ;;
        pretti)
            find ./src ./tests ./examples -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format --verbose -i {} +
            ;;
        *)
            "$@"
            ;;
    esac
}

printf "\t..: Using version $VERSION on $CONTEXT\n"
main "$@"

