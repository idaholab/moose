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

TEST(AbaqusInputParserTest, OptionDataMultipleLines)
{
  // Two separate data lines under a single option (no continuation)
  std::istringstream in(
      "*Boundary\n"
      "XSYMM, 1, 1, 1.0\n"
      "YSYMM, 2, 3, -2.5\n");

  InputParser parser;
  parser.parse(in);

  ASSERT_EQ(parser._children.size(), 1u);
  auto * opt = dynamic_cast<OptionNode *>(parser._children[0].get());
  ASSERT_NE(opt, nullptr);
  EXPECT_EQ(opt->_type, "boundary");

  ASSERT_EQ(opt->_data.size(), 2u);
  // First row
  ASSERT_GE(opt->_data[0].size(), 4u);
  EXPECT_EQ(opt->_data[0][0], "XSYMM");
  EXPECT_EQ(opt->_data[0][1], "1");
  EXPECT_EQ(opt->_data[0][2], "1");
  EXPECT_EQ(opt->_data[0][3], "1.0");
  // Second row
  ASSERT_GE(opt->_data[1].size(), 4u);
  EXPECT_EQ(opt->_data[1][0], "YSYMM");
  EXPECT_EQ(opt->_data[1][1], "2");
  EXPECT_EQ(opt->_data[1][2], "3");
  EXPECT_EQ(opt->_data[1][3], "-2.5");
}

TEST(AbaqusInputParserTest, NsetMultipleLines)
{
  // Two data lines under an Nset option inside an Assembly block
  std::istringstream in(
      "*Assembly, name=A\n"
      "*Nset, nset=ALL, instance=I\n"
      "1, 2\n"
      "3, 4\n"
      "*End Assembly\n");

  InputParser parser;
  parser.parse(in);

  auto * asmb = find_child<BlockNode>(parser, "assembly");
  ASSERT_NE(asmb, nullptr);
  auto * nset = find_child<OptionNode>(*asmb, "nset");
  ASSERT_NE(nset, nullptr);

  EXPECT_TRUE(nset->_header.has("nset"));
  EXPECT_EQ(nset->_header.get<std::string>("nset"), "ALL");
  EXPECT_TRUE(nset->_header.has("instance"));
  EXPECT_EQ(nset->_header.get<std::string>("instance"), "I");

  ASSERT_EQ(nset->_data.size(), 2u);
  ASSERT_EQ(nset->_data[0].size(), 2u);
  ASSERT_EQ(nset->_data[1].size(), 2u);
  EXPECT_EQ(nset->_data[0][0], "1");
  EXPECT_EQ(nset->_data[0][1], "2");
  EXPECT_EQ(nset->_data[1][0], "3");
  EXPECT_EQ(nset->_data[1][1], "4");
}

TEST(AbaqusInputParserTest, NsetContinuationAndMultipleLines)
{
  // First logical row uses a continuation to span two physical lines
  std::istringstream in(
      "*Assembly, name=A\n"
      "*Nset, nset=A, instance=I\n"
      "1, 2,\n"
      "3, 4\n"
      "5, 6\n"
      "*End Assembly\n");

  InputParser parser;
  parser.parse(in);

  auto * asmb = find_child<BlockNode>(parser, "assembly");
  ASSERT_NE(asmb, nullptr);
  auto * nset = find_child<OptionNode>(*asmb, "nset");
  ASSERT_NE(nset, nullptr);

  EXPECT_EQ(nset->_header.get<std::string>("nset"), "A");
  EXPECT_EQ(nset->_header.get<std::string>("instance"), "I");

  // After continuation joining, expect two logical data rows: [1,2,3,4] and [5,6]
  ASSERT_EQ(nset->_data.size(), 2u);
  ASSERT_EQ(nset->_data[0].size(), 4u);
  EXPECT_EQ(nset->_data[0][0], "1");
  EXPECT_EQ(nset->_data[0][1], "2");
  EXPECT_EQ(nset->_data[0][2], "3");
  EXPECT_EQ(nset->_data[0][3], "4");

  ASSERT_EQ(nset->_data[1].size(), 2u);
  EXPECT_EQ(nset->_data[1][0], "5");
  EXPECT_EQ(nset->_data[1][1], "6");
}

TEST(AbaqusInputParserTest, SquareInpTreeAndAssembly)
{
  // Minimal assembly-style content to avoid external data dependency
  std::istringstream in(
      "*Part, name=Cube\n"
      "*Node\n1, 0., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=1, Variables=1\n"
      "1\n"
      "*Element, Type=U1, Elset=CUBE\n"
      "1, 1\n"
      "*UEL PROPERTY, elset=CUBE\n"
      "1.0\n"
      "*End Part\n"
      "*Assembly, name=Assembly\n"
      "*Instance, name=Cube-1, part=Cube\n"
      "*End Instance\n"
      "*Nset, nset=ALL, instance=Cube-1\n"
      "1\n"
      "*End Assembly\n");

  InputParser parser;
  parser.parse(in);

  EXPECT_FALSE(parser.isFlat());

  auto * part = find_child<BlockNode>(parser, "part");
  auto * asmb = find_child<BlockNode>(parser, "assembly");
  ASSERT_NE(part, nullptr);
  ASSERT_NE(asmb, nullptr);

  EXPECT_NE(find_child<OptionNode>(*part, "node"), nullptr);
  EXPECT_NE(find_child<OptionNode>(*part, "user element"), nullptr);
  EXPECT_NE(find_child<OptionNode>(*part, "element"), nullptr);
  EXPECT_NE(find_child<OptionNode>(*part, "uel property"), nullptr);

  auto * inst = find_child<BlockNode>(*asmb, "instance");
  ASSERT_NE(inst, nullptr);

  auto * nset_any = find_child<OptionNode>(*asmb, "nset");
  ASSERT_NE(nset_any, nullptr);
  EXPECT_TRUE(nset_any->_header.has("nset"));
  EXPECT_TRUE(nset_any->_header.has("instance"));
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
