//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Moose.h"

#include "hit/parse.h"

TEST(MooseTest, hitMessagePrefix)
{
  const auto build_root = [](const auto & contents, const std::string & filename = "file")
  { return std::unique_ptr<const hit::Node>(hit::parse(filename, contents)); };

  // Invalid state (this is a wasp bug)
  {
    const auto root = build_root("");
    ASSERT_FALSE(Moose::hitMessagePrefix(*root).has_value());
  }
  // Root node so no line info
  {
    const auto root = build_root("foo=bar");
    const auto prefix = Moose::hitMessagePrefix(*root);
    ASSERT_TRUE(prefix.has_value());
    ASSERT_EQ(*prefix, root->filename() + ":");
  }
  // Command line argument root
  {
    const auto root = build_root("foo=bar", Moose::hit_command_line_filename);
    const auto prefix = Moose::hitMessagePrefix(*root);
    ASSERT_TRUE(prefix.has_value());
    ASSERT_EQ(*prefix, root->filename() + ":");
  }
  // Command line argument, without a fullpath
  {
    const auto root = build_root("foo=bar", Moose::hit_command_line_filename);
    const auto node = root->find("foo");
    ASSERT_NE(node, nullptr);
    const auto prefix = Moose::hitMessagePrefix(*node);
    ASSERT_TRUE(prefix.has_value());
    ASSERT_EQ(*prefix, root->filename() + ":");
  }
  // Command line argument, with a fullpath
  {
    const auto root = build_root("foo=bar", Moose::hit_command_line_filename);
    const auto node = root->find("foo");
    ASSERT_NE(node, nullptr);
    const auto prefix = Moose::hitMessagePrefix(*node, true);
    ASSERT_TRUE(prefix.has_value());
    ASSERT_EQ(*prefix, node->filename() + ":" + node->fullpath() + ":");
  }
  // Standard parameter with a fullpath
  {
    const auto root = build_root("foo=bar");
    const auto node = root->find("foo");
    ASSERT_NE(node, nullptr);
    const auto prefix = Moose::hitMessagePrefix(*node);
    ASSERT_TRUE(prefix.has_value());
    ASSERT_EQ(*prefix, node->fileLocation() + ":");
  }
}
