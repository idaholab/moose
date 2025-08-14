//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "AbaqusInputParser.h"
#include "DataFileUtils.h"

using namespace Abaqus;

namespace
{

// Utility: parse from a module data file under solid_mechanics:unit/
static void parse_from_unit_file(const std::string & rel_path, InputParser & parser)
{
  const auto file = Moose::DataFileUtils::getPath("solid_mechanics:unit/" + rel_path).path;
  std::ifstream in(file);
  ASSERT_TRUE(in.good()) << "Failed to open test input: " << file;
  parser.parse(in);
}

// Utility: find first child of a block by type string
template <typename NodeT>
static NodeT * find_child(Abaqus::BlockNode & node, const std::string & type)
{
  for (const auto & child : node._children)
    if (child && child->_type == type)
      return dynamic_cast<NodeT *>(child.get());
  return nullptr;
}

} // namespace

TEST(AbaqusInputParserTest, HeaderMapParsing)
{
  // Simulate a header line: "*Output, field, variable=PRESELECT, VALUE = 3"
  HeaderMap header({"*Output", "field", "variable=PRESELECT", "VALUE = 3"});

  // Presence-only key
  EXPECT_TRUE(header.has("field"));
  EXPECT_TRUE(header.get<bool>("field"));

  // Case-insensitive keys; values preserved (trimmed)
  EXPECT_TRUE(header.has("variable"));
  EXPECT_EQ(header.get<std::string>("variable"), "PRESELECT");

  EXPECT_TRUE(header.has("value"));
  EXPECT_EQ(header.get<int>("value"), 3);

  // Defaults and missing keys
  EXPECT_FALSE(header.has("missing"));
  EXPECT_EQ(header.get<int>("missing", 42), 42);
}

TEST(AbaqusInputParserTest, UnknownKeywordThrows)
{
  // Contains an unknown keyword *Foo that is neither a block nor an option
  std::istringstream in("*Heading\n** comment\n*Foo, bar=1\n");
  InputParser parser;
  try
  {
    parser.parse(in);
    FAIL() << "Expected an exception for unknown keyword";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Unknown keyword '*Foo'"), std::string::npos) << msg;
  }
}

TEST(AbaqusInputParserTest, ContinuationEOFThrows)
{
  // Trailing comma must be followed by a continuation line
  std::istringstream in("*Heading\n*Node\n1, 0., 0.,\n");
  InputParser parser;
  try
  {
    parser.parse(in);
    FAIL() << "Expected an exception for missing continuation line";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Expected a continuation line but got EOF"), std::string::npos) << msg;
  }
}

TEST(AbaqusInputParserTest, OptionDataAndContinuationJoin)
{
  // One option with two data lines; first ends with comma (continuation)
  std::istringstream in(
      "*Boundary\n"
      "XSYMM, 1, 1,\n"
      "1.0\n"
      "YSYMM, 2, 2\n");

  InputParser parser;
  parser.parse(in);

  // Root should have a single option child of type "boundary"
  ASSERT_EQ(parser._children.size(), 1u);
  auto * opt = dynamic_cast<OptionNode *>(parser._children[0].get());
  ASSERT_NE(opt, nullptr);
  EXPECT_EQ(opt->_type, "boundary");

  // Data lines are tokenized and trimmed; continuation was joined before tokenization
  ASSERT_EQ(opt->_data.size(), 2u);
  // First logical line contains the full continued data (",\n 1.0" joined)
  ASSERT_GE(opt->_data[0].size(), 4u);
  EXPECT_EQ(opt->_data[0][0], "XSYMM");
  EXPECT_EQ(opt->_data[0][1], "1");
  EXPECT_EQ(opt->_data[0][2], "1");
  EXPECT_EQ(opt->_data[0][3], "1.0");

  // Second data row
  ASSERT_GE(opt->_data[1].size(), 3u);
  EXPECT_EQ(opt->_data[1][0], "YSYMM");
  EXPECT_EQ(opt->_data[1][1], "2");
  EXPECT_EQ(opt->_data[1][2], "2");
}

TEST(AbaqusInputParserTest, SquareInpTreeAndAssembly)
{
  InputParser parser;
  parse_from_unit_file("square.inp", parser);

  // square.inp contains an Assembly block => not flat
  EXPECT_FALSE(parser.isFlat());

  // Verify we have a Part and an Assembly block at top level
  auto * part = find_child<BlockNode>(parser, "part");
  auto * asmb = find_child<BlockNode>(parser, "assembly");
  ASSERT_NE(part, nullptr);
  ASSERT_NE(asmb, nullptr);

  // Inside Part: expect options like node, user element, element, uel property
  EXPECT_NE(find_child<OptionNode>(*part, "node"), nullptr);
  EXPECT_NE(find_child<OptionNode>(*part, "user element"), nullptr);
  EXPECT_NE(find_child<OptionNode>(*part, "element"), nullptr);
  EXPECT_NE(find_child<OptionNode>(*part, "uel property"), nullptr);

  // Inside Assembly: expect an Instance sub-block and Nset options
  auto * inst = find_child<BlockNode>(*asmb, "instance");
  ASSERT_NE(inst, nullptr);

  auto * nset_all = find_child<OptionNode>(*asmb, "nset");
  ASSERT_NE(nset_all, nullptr);
  // Verify header parsing on the first Nset option
  EXPECT_TRUE(nset_all->_header.has("nset"));
  EXPECT_EQ(nset_all->_header.get<std::string>("nset"), "ALL");
  EXPECT_EQ(nset_all->_header.get<std::string>("instance"), "Cube-1");

  // It should have two data rows of node ids
  ASSERT_EQ(nset_all->_data.size(), 2u);
}

TEST(AbaqusInputParserTest, FlatFileDetection)
{
  // Minimal flat-style file without an Assembly block
  std::istringstream in(
      "*Part, name=P\n"
      "*Node\n1, 0., 0.\n"
      "*End Part\n"
      "*Step\n*Static\n1., 1.\n*End Step\n");

  InputParser parser;
  parser.parse(in);

  EXPECT_TRUE(parser.isFlat());
  // Has Part and Step blocks
  EXPECT_NE(find_child<BlockNode>(parser, "part"), nullptr);
  EXPECT_NE(find_child<BlockNode>(parser, "step"), nullptr);
}

