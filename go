#!/usr/bin/env bash
set -euo pipefail

CONTEXT=${CONTEXT:-local}
VERSION_TAG=$(git describe || git branch --show-current)
VERSION=${VERSION:-$VERSION_TAG}


BUILD_PARAMS="
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
-DBUILD_EXAMPLES=ON
"
TEST_PARAMS="
-DBUILD_TESTS=ON
"

build()
{
    extra_args=$@
    cmake ${BUILD_PARAMS} ${TEST_PARAMS} ${extra_args} -S . -B ./build
    cmake --build ./build -- -j$(nproc)
}


main()
{
    mkdir -p build
    case $1 in
        debug|release)
            export CMAKE_BUILD_TYPE=${goal^};
            EXTRA_CMAKE_ARGS=$@
            build $EXTRA_CMAKE_ARGS
            ;;
        clean)
            printf "Wiping ./build folder completely.\n"
            rm -rf ./build
            ;;
        check)
            build
            ctest --extra-verbose --output-on-failure --test-dir ./build/tests
            ;;
        run)
            build
            ./build/hello_ion
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

printf "\t..: Running version $VERSION on $CONTEXT\n"
main "$@"

