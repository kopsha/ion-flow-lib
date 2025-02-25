#!/usr/bin/env bash
set -euo pipefail

CONTEXT=${CONTEXT:-local}
VERSION_TAG=$(git describe || git branch --show-current)
export VERSION=${VERSION:-$VERSION_TAG}
HOST_OS=$(echo $(uname) | tr '[:upper:]' '[:lower:]')


build()
{
    mkdir -p build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DTARGET_PLATFORM=linux \
        -DTARGET_ABI=x86_64 \
        -S . -B ./build
    cmake --build ./build -- -j$(nproc)
}

build_dist()
{
    mkdir -p build/dist
    rm -rf build/dist/*
    extra_args=$@
    export CC=$(which clang)
    export CXX=$(which clang++)
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DTARGET_PLATFORM=linux \
        -DTARGET_ABI=x86_64 \
        ${extra_args} -S . -B ./build
    cmake --build ./build -- -j$(nproc)
    cmake --install ./build --prefix build/dist
}

build_example()
{
    mkdir -p build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DTARGET_PLATFORM="${HOST_OS}" \
        -DTARGET_ABI=x86_64 \
        -DBUILD_EXAMPLES=ON \
        -S . -B ./build
    cmake --build ./build -- -j$(nproc)
}

build_test()
{
    mkdir -p build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
        -DTARGET_PLATFORM=linux \
        -DTARGET_ABI=x86_64 \
        -S . -B ./build
    cmake --build ./build -- -j$(nproc)
}

build_test_cover()
{
    mkdir -p build/coverage
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
        -DTARGET_PLATFORM=linux \
        -DTARGET_ABI=x86_64 \
        -DINCLUDE_COVERAGE=ON \
        -S . -B ./build
    cmake --build ./build -- -j$(nproc)
}

main()
{

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
        cover)
            build_test_cover
            ctest --output-on-failure --test-dir ./build/tests
            cmake --build ./build --target coverage
            ;;
        run)
            build_example
            ./build/examples/ionExample
            ;;
        check)
            build_test
            ctest --output-on-failure --test-dir ./build/tests
            ;;
        tdd)
            build_test
            find ./src ./tests ./examples -name "*.cpp" -o -name "*.h" | entr -rc sh -c "cmake --build ./build -- -j$(nproc) && ctest --output-on-failure --test-dir ./build/tests"
            ;;
        pretti)
            find ./src ./tests ./examples -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format --verbose -i {} +
            ;;
        *)
            "$@"
            ;;
    esac
}

printf "\t..: Running version $VERSION on $CONTEXT\n"
main "$@"

