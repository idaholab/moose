//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "StringInputStream.h"

TEST(StringInputStreamTest, testGet)
{
  const std::string test_string = "it works!";

  auto input = std::make_unique<std::stringstream>();
  *input << test_string;
  StringInputStream sis(std::move(input));

  auto stream = sis.get();
  std::stringstream sstream;
  sstream << stream->rdbuf();
  EXPECT_EQ(sstream.str(), test_string);
}

TEST(StringInputStreamTest, testRelease)
{
  auto input = std::make_unique<std::stringstream>();
  auto input_ptr = input.get();
  StringInputStream sis(std::move(input));

  try
  {
    auto stream = sis.get();
    sis.release();
    FAIL();
  }
  catch (const std::exception & err)
  {
    const auto pos = std::string(err.what()).find("Cannot release");
    ASSERT_TRUE(pos != std::string::npos);
  }

  auto new_input = sis.release();
  EXPECT_EQ(new_input.get(), input_ptr);
}

TEST(StringInputStreamTest, testInUse)
{
  auto input = std::make_unique<std::stringstream>();
  StringInputStream sis(std::move(input));

  EXPECT_FALSE(sis.inUse());

  auto stream = sis.get();
  EXPECT_TRUE(sis.inUse());
}

TEST(StringInputStreamTest, testFilename)
{
  std::unique_ptr<std::stringstream> input = std::make_unique<std::stringstream>();
  StringInputStream sis(std::move(input));

  EXPECT_EQ(sis.getFilename(), std::optional<std::filesystem::path>{});
}
