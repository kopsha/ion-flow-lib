#include "ionflow.h"
#include "version.h"

#include <gtest/gtest.h>

TEST(Generics, buffers_cannot_have_size_0) { EXPECT_GT(BUFFER_SIZE, 0); }

TEST(Generics, version_must_exist) {
    EXPECT_FALSE(ionflow::version.empty());
}
