/**
 * @file main.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the main entry point for the unit tests.
 * @version 0.1
 * @date 2024-05-29
 * 
 * @copyright Copyright (c) 2024 Simon Cahill and Contributors.
 */

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}