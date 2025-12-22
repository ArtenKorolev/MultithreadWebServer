#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "IniParser.h"

using namespace webserver::core;

//
// BASIC CASES
//

TEST(IniParser, EmptyConfig) {
  IniParser parser("");
  auto cfg = parser.parse();
  EXPECT_TRUE(cfg.empty());
}

TEST(IniParser, OnlyComments) {
  IniParser parser(
      "# comment\n"
      "; another comment\n"
      "\n");

  auto cfg = parser.parse();
  EXPECT_TRUE(cfg.empty());
}

//
// SECTIONS
//

TEST(IniParser, SingleSectionSingleKey) {
  IniParser parser(
      "[server]\n"
      "port=8080\n");

  auto cfg = parser.parse();

  ASSERT_EQ(cfg.size(), 1);
  EXPECT_EQ(cfg["server.port"], "8080");
}

TEST(IniParser, SectionWithSpacesTrimmed) {
  IniParser parser(
      "[ server ]\n"
      "host = 127.0.0.1\n");

  auto cfg = parser.parse();
  EXPECT_EQ(cfg["server.host"], "127.0.0.1");
}

TEST(IniParser, RepeatedSectionAllowed) {
  IniParser parser(
      "[server]\n"
      "port=8080\n"
      "[server]\n"
      "host=localhost\n");

  auto cfg = parser.parse();

  EXPECT_EQ(cfg["server.port"], "8080");
  EXPECT_EQ(cfg["server.host"], "localhost");
}

//
// KEYS / VALUES
//

TEST(IniParser, TrimKeyAndValue) {
  IniParser parser(
      "[s]\n"
      "   key \t = \t value   \n");

  auto cfg = parser.parse();
  EXPECT_EQ(cfg["s.key"], "value");
}

TEST(IniParser, EmptyValueAllowed) {
  IniParser parser(
      "[s]\n"
      "key=\n");

  auto cfg = parser.parse();
  EXPECT_EQ(cfg["s.key"], "");
}

TEST(IniParser, DuplicateKeyOverridesPrevious) {
  IniParser parser(
      "[s]\n"
      "key=1\n"
      "key=2\n");

  auto cfg = parser.parse();
  EXPECT_EQ(cfg["s.key"], "2");
}

//
// ERROR HANDLING
//

TEST(IniParser, KeyOutsideSectionThrows) {
  IniParser parser("key=value\n");
  EXPECT_THROW(auto t = parser.parse(), std::runtime_error);
}

TEST(IniParser, InvalidSectionSyntaxThrows) {
  IniParser parser("[server\n");
  EXPECT_THROW(auto t =parser.parse(), std::runtime_error);
}

TEST(IniParser, InvalidKeyValueLineThrows) {
  IniParser parser(
      "[s]\n"
      "invalid_line\n");

  EXPECT_THROW(auto t =parser.parse(), std::runtime_error);
}

TEST(IniParser, EmptySectionNameThrows) {
  IniParser parser(
      "[]\n"
      "key=value\n");

  EXPECT_THROW(auto t = parser.parse(), std::runtime_error);
}

//
// PLATFORM / LINE ENDINGS
//

TEST(IniParser, WindowsCRLFLineEndings) {
  IniParser parser(
      "[s]\r\n"
      "key=value\r\n");

  auto cfg = parser.parse();
  EXPECT_EQ(cfg["s.key"], "value");
}

TEST(IniParser, NoTrailingNewline) {
  IniParser parser(
      "[s]\n"
      "key=value");

  auto cfg = parser.parse();
  EXPECT_EQ(cfg["s.key"], "value");
}