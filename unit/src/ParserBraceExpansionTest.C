//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Parser.h"
#include "gtest_include.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

std::unique_ptr<Parser>
parseInputs(const std::vector<std::pair<std::string, std::string>> & inputs,
            const std::vector<std::string> & cli_args = {})
{
  std::vector<std::string> filenames;
  std::vector<std::string> input_text;
  for (const auto & [filename, text] : inputs)
  {
    filenames.push_back(filename);
    input_text.push_back(text);
  }

  auto parser = std::make_unique<Parser>(filenames, input_text);
  parser->setThrowOnError(true);
  if (!cli_args.empty())
    parser->setCommandLineParams(cli_args);
  parser->parse();
  return parser;
}

hit::Field &
requireField(hit::Node & root, const std::string & path)
{
  auto * node = root.find(path);
  if (!node)
    throw std::runtime_error("missing field '" + path + "'");

  auto * field = dynamic_cast<hit::Field *>(node);
  if (!field)
    throw std::runtime_error("node '" + path + "' is not a field");

  return *field;
}

} // namespace

TEST(ParserBraceExpansionTest, SubstitutesCommandLineParameter)
{
  auto parser =
      parseInputs({{"main.i", "file_base = out_${FILENAME}\n"}}, {"FILENAME=special_string"});
  EXPECT_EQ(requireField(parser->getRoot(), "file_base").strVal(), "out_special_string");
}

TEST(ParserBraceExpansionTest, SubstitutesInFileParameter)
{
  auto parser =
      parseInputs({{"main.i", "FILENAME = 'special_string'\nfile_base = out_${FILENAME}\n"}});
  EXPECT_EQ(requireField(parser->getRoot(), "file_base").strVal(), "out_special_string");
}

TEST(ParserBraceExpansionTest, ExpandsUnitsConversions)
{
  auto parser = parseInputs({{"units.i",
                              "km_to_m = ${units 1 km -> m}\n"
                              "Jmol_to_eVat = ${units 1 J/mol -> eV/at}\n"
                              "mW = ${units 3 mW}\n"}});

  EXPECT_DOUBLE_EQ(requireField(parser->getRoot(), "km_to_m").floatVal(), 1000.0);
  EXPECT_DOUBLE_EQ(requireField(parser->getRoot(), "Jmol_to_eVat").floatVal(),
                   1.0364269656262175e-05);
  EXPECT_DOUBLE_EQ(requireField(parser->getRoot(), "mW").floatVal(), 3.0);
}

TEST(ParserBraceExpansionTest, ExpandsNestedBraceExpressions)
{
  auto parser = parseInputs(
      {{"nested.i",
        "foo1 = 41\n"
        "foo2 = 42\n"
        "num = 1\n"
        "x = 2\n"
        "prefix_foo1 = 7\n"
        "implicit_replace_nested_cmd = ${${raw re place} ${raw foo ${num}}}\n"
        "nested_fparse = ${fparse ${replace ${raw foo ${num}}} + ${foo2}}\n"
        "nested_token_replace = ${replace prefix_${raw foo ${num}}}\n"
        "nested_units = ${fparse ${units ${fparse ${units ${x} m -> cm} / 2} cm -> m} + 1}\n"}});

  auto & root = parser->getRoot();
  EXPECT_EQ(requireField(root, "implicit_replace_nested_cmd").strVal(), "41");
  EXPECT_DOUBLE_EQ(requireField(root, "nested_fparse").floatVal(), 83.0);
  EXPECT_EQ(requireField(root, "nested_token_replace").strVal(), "7");
  EXPECT_DOUBLE_EQ(requireField(root, "nested_units").floatVal(), 2.0);
}

TEST(ParserBraceExpansionTest, ResolvesForwardBraceDependencies)
{
  auto parser = parseInputs({{"forward.i", "bar = ${foo}\nfoo = ${raw 4 1}\n"}});

  auto & root = parser->getRoot();
  EXPECT_EQ(requireField(root, "foo").strVal(), "41");
  EXPECT_EQ(requireField(root, "bar").strVal(), "41");
}

TEST(ParserBraceExpansionTest, ExpandsDeepNestedFparseAndUnits)
{
  auto parser = parseInputs(
      {{"deep.i",
        "deep_nested_fparse_units = "
        "${units ${fparse ${units ${fparse ${units ${fparse ${units ${fparse ${units ${fparse 1 + "
        "1} m -> cm} / 2} cm -> m} * 3} m -> cm} + 4} cm -> m} * 5} m -> cm}\n"}});

  EXPECT_DOUBLE_EQ(requireField(parser->getRoot(), "deep_nested_fparse_units").floatVal(), 1520.0);
}

TEST(ParserBraceExpansionTest, ReportsCyclicBraceDependencies)
{
  try
  {
    parseInputs({{"cycle.i", "foo = ${bar}\nbar = ${foo}\n"}});
    FAIL() << "missing expected parse error";
  }
  catch (const Parser::Error & err)
  {
    EXPECT_NE(std::string(err.what()).find("cyclic brace-expression dependency detected"),
              std::string::npos);
  }
}

TEST(ParserBraceExpansionTest, ReportsUnitConversionsInDebugMode)
{
#ifdef NDEBUG
  GTEST_SKIP() << "requires a debug build";
#else
  testing::internal::CaptureStdout();
  parseInputs({{"debug_units.i", "value = ${units 1 J/mol -> eV/at}\n"}});
  const auto output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("Unit conversion 1 J/mol -> 1.0364269656262175e-05 eV/at"),
            std::string::npos);
#endif
}
