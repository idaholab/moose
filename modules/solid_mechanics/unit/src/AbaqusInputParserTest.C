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
#include "AbaqusInputObjects.h"
#include "DataFileUtils.h"

using namespace Abaqus;

namespace
{

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
  std::istringstream in("*Heading\n*Node\n1, 0., 0.,");
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

TEST(AbaqusInputParserTest, AssemblyScopedSetsAndIndices)
{
  // Build a minimal assembly with an instance and both header- and inline-scoped sets
  std::istringstream in(
      "*Part, name=Cube\n"
      "*Node\n"
      "1, 0., 0.\n"
      "2, 1., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=1, Variables=1\n"
      "1\n"
      "*Element, Type=U1, Elset=CUBE\n"
      "1, 1\n"
      "*End Part\n"
      "*Assembly, name=Assembly\n"
      "*Instance, name=Cube-1, part=Cube\n"
      "*End Instance\n"
      // Header-scoped set using clean integers
      "*Nset, nset=ALL, instance=Cube-1\n"
      "1, 2\n"
      // Inline instance-qualified set
      "*Nset, nset=INLINE\n"
      "Cube-1.1, Cube-1.2\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  model.parse(parser);

  ASSERT_TRUE(model._assembly);
  ASSERT_TRUE(model._assembly->_instance.has("Cube-1"));
  const auto & inst = model.getInstance("Cube-1");

  // Node index resolution: inline vs instance parameter
  const auto idx_inline = model.getNodeIndex("Cube-1.2");
  const auto idx_scoped = model.getNodeIndex("2", &inst);
  EXPECT_EQ(idx_inline, idx_scoped);

  // Element index resolution mirrors node logic
  const auto eidx_inline = model.getElementIndex("Cube-1.1");
  const auto eidx_scoped = model.getElementIndex("1", &inst);
  EXPECT_EQ(eidx_inline, eidx_scoped);

  // Header-scoped set populated
  ASSERT_TRUE(model._nsets.find("ALL") != model._nsets.end());
  const auto & all = model._nsets.at("ALL");
  ASSERT_EQ(all.size(), 2u);
  EXPECT_NE(std::find(all.begin(), all.end(), model.getNodeIndex("1", &inst)), all.end());
  EXPECT_NE(std::find(all.begin(), all.end(), model.getNodeIndex("2", &inst)), all.end());

  // Inline-scoped set populated
  ASSERT_TRUE(model._nsets.find("INLINE") != model._nsets.end());
  const auto & inline_set = model._nsets.at("INLINE");
  ASSERT_EQ(inline_set.size(), 2u);
  EXPECT_NE(std::find(inline_set.begin(), inline_set.end(), model.getNodeIndex("Cube-1.1")),
            inline_set.end());
  EXPECT_NE(std::find(inline_set.begin(), inline_set.end(), model.getNodeIndex("Cube-1.2")),
            inline_set.end());
}

TEST(AbaqusInputParserTest, AssemblyScopedSetsAmbiguityError)
{
  // Both header instance= and inline instance-qualified tokens -> ambiguous
  std::istringstream in(
      "*Part, name=Cube\n"
      "*Node\n"
      "1, 0., 0.\n"
      "*End Part\n"
      "*Assembly, name=Assembly\n"
      "*Instance, name=Cube-1, part=Cube\n"
      "*End Instance\n"
      "*Nset, nset=BAD, instance=Cube-1\n"
      "Cube-1.1\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  try
  {
    model.parse(parser);
    FAIL() << "Expected ambiguous instance scoping error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Ambiguous set token"), std::string::npos) << msg;
  }
}

TEST(AbaqusInputParserTest, AssemblyElsetToNsetCopyWithInstance)
{
  // Element set in part, assembly-level nodeset created from elset with instance scoping
  std::istringstream in(
      "*Part, name=Cube\n"
      "*Node\n"
      "1, 0., 0.\n"
      "2, 1., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=2, Variables=1\n"
      "1, 2\n"
      "*Element, Type=U1, Elset=CUBE\n"
      "1, 1, 2\n"
      "*End Part\n"
      "*Assembly, name=Assembly\n"
      "*Instance, name=Cube-1, part=Cube\n"
      "*End Instance\n"
      // Create a nodeset from the element set, assembly-level with instance
      "*Nset, nset=NODES_FROM_E, elset=CUBE, instance=Cube-1\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  model.parse(parser);

  ASSERT_TRUE(model._nsets.find("NODES_FROM_E") != model._nsets.end());
  const auto & ns = model._nsets.at("NODES_FROM_E");
  ASSERT_EQ(ns.size(), 2u);

  const auto & inst = model.getInstance("Cube-1");
  const auto n1 = model.getNodeIndex("1", &inst);
  const auto n2 = model.getNodeIndex("2", &inst);
  EXPECT_NE(std::find(ns.begin(), ns.end(), n1), ns.end());
  EXPECT_NE(std::find(ns.begin(), ns.end(), n2), ns.end());
}

TEST(AbaqusInputParserTest, AssemblyNsetRequiresInstance)
{
  // Assembly-level nodeset must specify an instance (header or inline). Numeric-only should error.
  std::istringstream in(
      "*Part, name=Cube\n"
      "*Node\n"
      "1, 0., 0.\n"
      "2, 1., 0.\n"
      "*End Part\n"
      "*Assembly, name=Assembly\n"
      "*Instance, name=Cube-1, part=Cube\n"
      "*End Instance\n"
      "*Nset, nset=BAD\n"
      "1, 2\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  try
  {
    model.parse(parser);
    FAIL() << "Expected error due to missing instance for assembly-level *Nset";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("requires an instance"), std::string::npos) << msg;
  }
}

TEST(AbaqusInputParserTest, MultipleInstances_AssemblyLevelSets)
{
  // Two instances of the same part; create assembly-level sets mixing nodes from both
  std::istringstream in(
      "*Part, name=P\n"
      "*Node\n"
      "1, 0., 0.\n"
      "2, 1., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=2, Variables=1\n"
      "1, 2\n"
      "*Element, Type=U1, Elset=EALL\n"
      "1, 1, 2\n"
      "*End Part\n"
      "*Assembly, name=A\n"
      "*Instance, name=I1, part=P\n"
      "*End Instance\n"
      "*Instance, name=I2, part=P\n"
      "*End Instance\n"
      // Header-scoped nodeset for I1
      "*Nset, nset=NS_I1, instance=I1\n"
      "1, 2\n"
      // Mixed inline instance-qualified nodeset
      "*Nset, nset=NS_MIXED\n"
      "I1.1, I2.2\n"
      // Assembly-level elset for I2 via inline instance-qualified element id
      "*Elset, elset=ES_I2\n"
      "I2.1\n"
      // Create nodeset from elset at assembly scope for I2
      "*Nset, nset=NS_FROM_ES, elset=EALL, instance=I2\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  model.parse(parser);

  // Instances exist
  const auto & i1 = model.getInstance("I1");
  const auto & i2 = model.getInstance("I2");

  // Header-scoped nodeset indices resolve within I1
  ASSERT_TRUE(model._nsets.find("NS_I1") != model._nsets.end());
  const auto & ns_i1 = model._nsets.at("NS_I1");
  ASSERT_EQ(ns_i1.size(), 2u);
  EXPECT_NE(std::find(ns_i1.begin(), ns_i1.end(), model.getNodeIndex("1", &i1)), ns_i1.end());
  EXPECT_NE(std::find(ns_i1.begin(), ns_i1.end(), model.getNodeIndex("2", &i1)), ns_i1.end());

  // Mixed inline nodeset contains nodes from different instances
  ASSERT_TRUE(model._nsets.find("NS_MIXED") != model._nsets.end());
  const auto & ns_mixed = model._nsets.at("NS_MIXED");
  ASSERT_EQ(ns_mixed.size(), 2u);
  EXPECT_NE(std::find(ns_mixed.begin(), ns_mixed.end(), model.getNodeIndex("I1.1")),
            ns_mixed.end());
  EXPECT_NE(std::find(ns_mixed.begin(), ns_mixed.end(), model.getNodeIndex("I2.2")),
            ns_mixed.end());

  // Assembly-level elset for I2 and derived nodeset from that elset
  ASSERT_TRUE(model._elsets.find("ES_I2") != model._elsets.end());
  const auto & es_i2 = model._elsets.at("ES_I2");
  ASSERT_EQ(es_i2.size(), 1u);
  EXPECT_NE(std::find(es_i2.begin(), es_i2.end(), model.getElementIndex("1", &i2)), es_i2.end());

  ASSERT_TRUE(model._nsets.find("NS_FROM_ES") != model._nsets.end());
  const auto & ns_from_es = model._nsets.at("NS_FROM_ES");
  ASSERT_EQ(ns_from_es.size(), 2u);
  EXPECT_NE(std::find(ns_from_es.begin(), ns_from_es.end(), model.getNodeIndex("1", &i2)),
            ns_from_es.end());
  EXPECT_NE(std::find(ns_from_es.begin(), ns_from_es.end(), model.getNodeIndex("2", &i2)),
            ns_from_es.end());
}

TEST(AbaqusInputParserTest, MultipleInstances_PartLevelSetsMerged)
{
  // Part-level sets should merge across instances into model-level sets with proper offsets
  std::istringstream in(
      "*Part, name=P\n"
      "*Node\n"
      "1, 0., 0.\n"
      "2, 1., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=2, Variables=1\n"
      "1, 2\n"
      "*Element, Type=U1, Elset=EALL\n"
      "1, 1, 2\n"
      // Define part-level sets
      "*Nset, nset=PN\n"
      "1, 2\n"
      // Define a part-level elset by referencing an existing one
      "*Elset, elset=PE\n"
      "EALL\n"
      "*End Part\n"
      "*Assembly, name=A\n"
      "*Instance, name=I1, part=P\n"
      "*End Instance\n"
      "*Instance, name=I2, part=P\n"
      "*End Instance\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  model.parse(parser);

  const auto & i1 = model.getInstance("I1");
  const auto & i2 = model.getInstance("I2");

  // Part-level nodeset PN should contain nodes from both instances
  ASSERT_TRUE(model._nsets.find("PN") != model._nsets.end());
  const auto & pn = model._nsets.at("PN");
  ASSERT_EQ(pn.size(), 4u);
  EXPECT_NE(std::find(pn.begin(), pn.end(), model.getNodeIndex("1", &i1)), pn.end());
  EXPECT_NE(std::find(pn.begin(), pn.end(), model.getNodeIndex("2", &i1)), pn.end());
  EXPECT_NE(std::find(pn.begin(), pn.end(), model.getNodeIndex("1", &i2)), pn.end());
  EXPECT_NE(std::find(pn.begin(), pn.end(), model.getNodeIndex("2", &i2)), pn.end());

  // Part-level elset PE should contain elements from both instances
  ASSERT_TRUE(model._elsets.find("PE") != model._elsets.end());
  const auto & pe = model._elsets.at("PE");
  ASSERT_EQ(pe.size(), 2u);
  EXPECT_NE(std::find(pe.begin(), pe.end(), model.getElementIndex("1", &i1)), pe.end());
  EXPECT_NE(std::find(pe.begin(), pe.end(), model.getElementIndex("1", &i2)), pe.end());
}

TEST(AbaqusInputParserTest, MultipleInstances_PartLevelElsetNumericMerged)
{
  // Part-level elset defined numerically should merge across instances
  std::istringstream in(
      "*Part, name=P\n"
      "*Node\n"
      "1, 0., 0.\n"
      "2, 1., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=2, Variables=1\n"
      "1, 2\n"
      "*Element, Type=U1, Elset=EALL\n"
      "1, 1, 2\n"
      // Define part-level elset numerically (using GENERATE to ensure robust parsing)
      "*Elset, elset=PE, generate\n"
      "1, 1, 1\n"
      "*End Part\n"
      "*Assembly, name=A\n"
      "*Instance, name=I1, part=P\n"
      "*End Instance\n"
      "*Instance, name=I2, part=P\n"
      "*End Instance\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  model.parse(parser);

  const auto & i1 = model.getInstance("I1");
  const auto & i2 = model.getInstance("I2");

  // Part-level elset exists and contains the single element index
  ASSERT_TRUE(model._part.has("P"));
  const auto & part = model._part["P"];
  ASSERT_FALSE(part._element_id_to_index.empty());
  ASSERT_TRUE(part._element_id_to_index.find(1) != part._element_id_to_index.end());
  ASSERT_TRUE(part._elsets.find("EALL") != part._elsets.end());
  const auto & eall_part = part._elsets.at("EALL");
  ASSERT_EQ(eall_part.size(), 1u);
  ASSERT_TRUE(part._elsets.find("PE") != part._elsets.end());
  const auto & pe_part = part._elsets.at("PE");
  ASSERT_EQ(pe_part.size(), 1u);

  ASSERT_TRUE(model._elsets.find("PE") != model._elsets.end());
  const auto & pe = model._elsets.at("PE");
  ASSERT_EQ(pe.size(), 2u);
  EXPECT_NE(std::find(pe.begin(), pe.end(), model.getElementIndex("1", &i1)), pe.end());
  EXPECT_NE(std::find(pe.begin(), pe.end(), model.getElementIndex("1", &i2)), pe.end());
}

// Intentionally expected-to-fail test capturing the bare numeric elset issue.
// A part-level *Elset with a simple numeric data line (no GENERATE) should populate the set,
// but currently the option data lines are not captured (option._data.size() == 0 observed),
// yielding an empty set. This test asserts the intended behavior and will fail until fixed.
TEST(AbaqusInputParserTest, PartLevelElsetNumericBareLine)
{
  std::istringstream in(
      "*Part, name=P\n"
      "*Node\n"
      "1, 0., 0.\n"
      "2, 1., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=2, Variables=1\n"
      "1, 2\n"
      "*Element, Type=U1, Elset=EALL\n"
      "1, 1, 2\n"
      // Bare numeric element set definition (no GENERATE)
      "*Elset, elset=PE\n"
      "1\n"
      "*End Part\n"
      "*Assembly, name=A\n"
      "*Instance, name=I1, part=P\n"
      "*End Instance\n"
      "*End Assembly\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  model.parse(parser);

  ASSERT_TRUE(model._part.has("P"));
  const auto & part = model._part["P"];

  // EXPECTED: PE should contain element id 1 resolved to the part-local index.
  ASSERT_TRUE(part._elsets.find("PE") != part._elsets.end());
  const auto & pe_part = part._elsets.at("PE");
  // This currently fails (size()==0); keep as 1 to encode intended behavior.
  ASSERT_EQ(pe_part.size(), 1u);
}

TEST(AbaqusInputParserTest, BoundaryInstanceScopedSingleNode)
{
  // Boundary with instance-scoped single node should resolve clean integer within that instance
  std::istringstream in(
      "*Part, name=Cube\n"
      "*Node\n"
      "1, 0., 0.\n"
      "*User Element, Type=U1, Coordinates=2, Nodes=1, Variables=1\n"
      "1\n"
      "*Element, Type=U1, Elset=CUBE\n"
      "1, 1\n"
      "*End Part\n"
      "*Assembly, name=Assembly\n"
      "*Instance, name=Cube-1, part=Cube\n"
      "*End Instance\n"
      "*End Assembly\n"
      "*Step, name=S1\n"
      "*Static\n"
      "1., 1.\n"
      "*Boundary, instance=Cube-1\n"
      "1, 1, 1, 7.5\n"
      "*End Step\n");

  Abaqus::InputParser parser;
  parser.parse(in);

  Abaqus::AssemblyModel model;
  model.parse(parser);

  ASSERT_EQ(model._step.size(), 1u);
  const auto & step = model._step[0];
  const auto & inst = model.getInstance("Cube-1");
  const auto idx = model.getNodeIndex("1", &inst);

  // DOF id = 1 should have value 7.5 at the resolved node index
  ASSERT_TRUE(step._bc_var_node_value_map.find(1) != step._bc_var_node_value_map.end());
  const auto & node_value = step._bc_var_node_value_map.at(1);
  ASSERT_TRUE(node_value.find(idx) != node_value.end());
  EXPECT_DOUBLE_EQ(node_value.at(idx), 7.5);
}
