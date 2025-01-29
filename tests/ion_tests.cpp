#include "ionflow/ionflow.h"
#include <gtest/gtest.h>

TEST(HelloTest, JustDoIt)
{
    EXPECT_EQ(getGreeting(), "Hello, Ions are flowing!");
}


TEST(MaxTest, TypicalCase)
{
    EXPECT_EQ(max(0, 1), 1);
    EXPECT_EQ(max(1, 0), 1);
}
