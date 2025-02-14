#include "ionflow.h"
#include <gtest/gtest.h>


TEST(BufferTests, Cannot_have_size_0)
{
    EXPECT_GT(BUFFER_SIZE, 0);
}
