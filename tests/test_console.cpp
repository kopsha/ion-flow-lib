#include "console.h"
#include <gtest/gtest.h>

TEST(ConsoleTest, can_call_functions_without_format_args)
{
    console::debug("anything");
    console::info("anything goes");
    console::warning("anything goes dangerous");
    console::error("if you don't know your thing");
}

TEST(Console, can_call_functions_with_format_args)
{
    console::debug("anything can go {}", "twice");
    console::info("anything goes {} times already", 2);
    console::warning("anything over {} goes dangerously {}", 10, "fast");
    console::error("if you don't make {} your thing what are you doing?", "templates");
}
