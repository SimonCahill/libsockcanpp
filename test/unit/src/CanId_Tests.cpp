/**
 * @file CanId_Tests.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains all the unit tests for the CanId structure.
 * @version 0.1
 * @date 2024-05-29
 * 
 * @copyright Copyright (c) 2024 Simon Cahill and Contributors.
 */

#include <gtest/gtest.h>

#include <CanId.hpp>

using sockcanpp::CanId;

using std::string;

TEST(CanIdTests, CanId_invalidId_ExpectFalse) {
    ASSERT_FALSE(CanId::isValidIdentifier(-1));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId) {
    ASSERT_TRUE(CanId::isValidIdentifier(0x123));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId) {
    ASSERT_TRUE(CanId::isValidIdentifier(0x123456));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ExplicitCast) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier((int32_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ExplicitCast) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier((int32_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ImplicitCast) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ImplicitCast) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ExplicitCastToInt16) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier((int16_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ExplicitCastToUint16) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier((uint16_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ExplicitCastToInt32) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier((int32_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ImplicitCastToInt16) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ImplicitCastToUint16) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ImplicitCastToInt32) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ExplicitCastToInt16) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier((int16_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ExplicitCastToUint16) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier((uint16_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ExplicitCastToInt32) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier((int32_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ImplicitCastToInt16) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ImplicitCastToUint16) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ImplicitCastToInt32) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ExplicitCastToUint32) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier((uint32_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ExplicitCastToUint32) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier((uint32_t)id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_StandardFrameId_ImplicitCastToUint32) {
    CanId id(0x123);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_validId_ExpectTrue_ExtendedFrameId_ImplicitCastToUint32) {
    CanId id(0x123456);
    ASSERT_TRUE(CanId::isValidIdentifier(id));
}

TEST(CanIdTests, CanId_isErrorFrame_ExpectTrue) {
    auto id = 0xe0000abc;
    ASSERT_TRUE(CanId::isErrorFrame(id));
}

TEST(CanIdTests, CanId_isErrorFrame_ExpectFalse) {
    auto id = 0x123;
    ASSERT_FALSE(CanId::isErrorFrame(id));
}

TEST(CanIdTests, CanId_isErrorFrame_ExpectTrue_ExplicitCast) {
    auto id = 0xe0000abc;
    ASSERT_TRUE(CanId::isErrorFrame((int32_t)id));
}

TEST(CanIdTests, CanId_isErrorFrame_ExpectFalse_ExplicitCast) {
    auto id = 0x123;
    ASSERT_FALSE(CanId::isErrorFrame((int32_t)id));
}

TEST(CanIdTests, CanId_isExtendedFrame_ExpectTrue) {
    auto id = 0xe0000abc;
    ASSERT_TRUE(CanId::isExtendedFrame(id));
}

TEST(CanIdTests, CanId_isExtendedFrame_ExpectFalse) {
    auto id = 0x123;
    ASSERT_FALSE(CanId::isExtendedFrame(id));
}

TEST(CanIdTests, CanId_isRtr_ExpectTrue) {
    auto id = 0x40000000;
    ASSERT_TRUE(CanId::isRemoteTransmissionRequest(id));
}

TEST(CanIdTests, CanId_isRtr_ExpectFalse) {
    auto id = 0x123;
    ASSERT_FALSE(CanId::isRemoteTransmissionRequest(id));
}

// Test constexpr
TEST(CanIdTests, CanId_isErrorFrame_ExpectTrue_Constexpr) {
    constexpr auto id = 0xe0000abc;
    ASSERT_TRUE(CanId::isErrorFrame(id));
}

TEST(CanIdTests, CanId_isErrorFrame_ExpectFalse_Constexpr) {
    constexpr auto id = 0x123;
    ASSERT_FALSE(CanId::isErrorFrame(id));
}

// Test implicit operators
// Implicit operators strip out control bits

TEST(CanIdTests, CanId_ImplicitOperatorInt32_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ((int32_t)id, 0x123);
}

TEST(CanIdTests, CanId_ImplicitOperatorInt16_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ((int16_t)id, 0x123);
}

TEST(CanIdTests, CanId_ImplicitOperatorUint16_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ((uint16_t)id, 0x123);
}

TEST(CanIdTests, CanId_ImplicitOperatorUint32_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ((uint32_t)id, 0x123);
}

// Test implicit operators with control bits set; these should be stripped.
TEST(CanIdTests, CanId_ImplicitOperatorInt32_ExpectTrue_ControlBits) {
    CanId id(0x12345678);
    ASSERT_EQ((int32_t)id, 0x12345678);
}

TEST(CanIdTests, CanId_ImplicitOperatorInt16_ExpectTrue_ControlBits) {
    CanId id(0x12345678);
    ASSERT_EQ((int16_t)id, 0x5678);
}

TEST(CanIdTests, CanId_ImplicitOperatorUint16_ExpectTrue_ControlBits) {
    CanId id(0x12345678);
    ASSERT_EQ((uint16_t)id, 0x5678);
}

TEST(CanIdTests, CanId_ImplicitOperatorUint32_ExpectTrue_ControlBits) {
    CanId id(0x12345678);
    ASSERT_EQ((uint32_t)id, 0x12345678);
}

// Test arithmetic operators
TEST(CanIdTests, CanId_ArithmeticOperatorPlus_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ(id + 0x123, 0x246);
}

TEST(CanIdTests, CanId_ArithmeticOperatorMinus_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ(id - 0x123, 0);
}

TEST(CanIdTests, CanId_ArithmeticOperatorPlusEquals_ExpectTrue) {
    CanId id(0x123);
    id += 0x123;
    ASSERT_EQ(id, 0x246);
}

TEST(CanIdTests, CanId_ArithmeticOperatorMinusEquals_ExpectTrue) {
    CanId id(0x123);
    id -= 0x123;
    ASSERT_EQ(id, 0);
}

TEST(CanIdTests, CanId_ArithmeticOperatorStar_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ(id * 2, 0x246);
}

TEST(CanIdTests, CanId_ArithmeticOperatorSlash_ExpectTrue) {
    CanId id(0x246);
    ASSERT_EQ(id / 2, 0x123);
}

TEST(CanIdTests, CanId_ArithmeticOperatorStarEquals_ExpectTrue) {
    CanId id(0x123);
    id *= 2;
    ASSERT_EQ(id, 0x246);
}

TEST(CanIdTests, CanId_ArithmeticOperatorSlashEquals_ExpectTrue) {
    CanId id(0x246);
    id /= 2;
    ASSERT_EQ(id, 0x123);
}

TEST(CanIdTests, CanId_ArithmeticOperatorModulo_ExpectTrue) {
    CanId id(0x123);
    ASSERT_EQ(id % 2, 1);
}

TEST(CanIdTests, CanId_ArithmeticOperatorModuloEquals_ExpectTrue) {
    CanId id(0x123);
    id %= 2;
    ASSERT_EQ(id, 1);
}

#if __cpp_concepts >= 201907

TEST(CanIdTests, CanId_TestStringToCanIdConversion_ExpectTrue) {
    string id{"0x123"};

    EXPECT_NO_THROW(CanId canId(id));
    EXPECT_NO_THROW(CanId canId{id});
    EXPECT_NO_THROW(CanId canId = id; (void)canId;);
    EXPECT_NO_THROW(CanId canId{"0x123"});

    CanId canId(id);
    ASSERT_EQ(canId, 0x123);
}

TEST(CanIdTests, CanId_TestStringToCanIdConversion_ExpectTrue_ExplicitCast) {
    string id{"0x123"};

    EXPECT_NO_THROW(CanId canId(static_cast<const char*>(id.c_str())));
    EXPECT_NO_THROW(CanId canId = static_cast<const char*>(id.c_str()); (void)canId;);
    EXPECT_NO_THROW(CanId canId = static_cast<const char*>(id.c_str()); (void)canId;);

    CanId canId(static_cast<const char*>(id.c_str()));
    ASSERT_EQ(canId, 0x123);
}

TEST(CanIdTests, CanId_TestStringToCanIdConversion_ExpectTrue_ImplicitCast) {
    string id{"0x123"};

    EXPECT_NO_THROW(CanId canId = id; (void)canId;);
    EXPECT_NO_THROW(CanId canId = id; (void)canId;);

    CanId canId = id;
    ASSERT_EQ(canId, 0x123);
}

TEST(CanIdTests, CanId_TestStringToCanIdConversion_ExpectError) {
    string id{"hello_world"};

    EXPECT_THROW(CanId canId(id), std::invalid_argument);
    EXPECT_THROW(CanId canId = id; (void)canId;, std::invalid_argument);
    EXPECT_THROW(CanId canId = id; (void)canId;, std::invalid_argument);
}

#endif // __cpp_concepts >= 201907