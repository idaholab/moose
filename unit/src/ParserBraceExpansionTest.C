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

namespace
{

/// Parses the given input, expecting it to fail, and returns the reported error messages
std::vector<hit::ErrorMessage>
parseErrors(const std::string & input)
{
  try
  {
    parseInputs({{"errors.i", input}});
  }
  catch (const Parser::Error & err)
  {
    return err.error_messages;
  }
  ADD_FAILURE() << "missing expected parse error";
  return {};
}

/// Expects \p error to contain \p message and to span [start_column, end_column] on \p line
void
expectErrorAt(const hit::ErrorMessage & error,
              const std::string & message,
              int line,
              int start_column,
              int end_column)
{
  EXPECT_NE(error.message.find(message), std::string::npos) << error.message;
  ASSERT_TRUE(error.lineinfo.has_value()) << error.message;
  EXPECT_EQ(error.lineinfo->start_line, line) << error.message;
  EXPECT_EQ(error.lineinfo->start_column, start_column) << error.message;
  EXPECT_EQ(error.lineinfo->end_column, end_column) << error.message;
}

} // namespace

TEST(ParserBraceExpansionTest, ParsesNestedBraceTemplate)
{
  const auto root = hit::parseBraceTemplate("a${b ${c}x}d");

  ASSERT_EQ(root.parts().size(), 3);
  EXPECT_EQ(std::get<std::string>(root.parts()[0]), "a");
  EXPECT_EQ(std::get<std::string>(root.parts()[2]), "d");
  EXPECT_FALSE(root.pureExpression());
  EXPECT_EQ(root.expressionCount(), 1);

  const auto & outer = *std::get<std::shared_ptr<hit::BraceExpr>>(root.parts()[1]);
  EXPECT_EQ(outer.text(), "${b ${c}x}");
  EXPECT_EQ(outer.position(), 1);
  EXPECT_EQ(outer.length(), 10);
  EXPECT_EQ(outer.column(), 2);
  ASSERT_EQ(outer.args().size(), 2);
  EXPECT_EQ(std::get<std::string>(outer.args()[0].value().parts()[0]), "b");

  // the second argument is a nested expression followed by literal text
  const auto & arg = outer.args()[1].value();
  ASSERT_EQ(arg.parts().size(), 2);
  const auto & inner = *std::get<std::shared_ptr<hit::BraceExpr>>(arg.parts()[0]);
  EXPECT_EQ(inner.text(), "${c}");
  EXPECT_EQ(inner.position(), 5);
  EXPECT_EQ(std::get<std::string>(arg.parts()[1]), "x");
}

TEST(ParserBraceExpansionTest, ReportsUnclosedBraceTemplate)
{
  hit::BraceParseError error;
  EXPECT_THROW(hit::parseBraceTemplate("x ${fparse ${a} + 1", &error), hit::Error);
  EXPECT_EQ(error.message, "missing closing '}' in brace expression");
  EXPECT_EQ(error.line, 1);
  EXPECT_EQ(error.column, 20);
}

TEST(ParserBraceExpansionTest, PropagatesFieldKinds)
{
  auto parser = parseInputs({{"kinds.i",
                              "x = 3\n"
                              "pure_float = ${fparse 1 + 2}\n"
                              "pure_int = ${x}\n"
                              "prefixed = foo_${x}\n"
                              "two_exprs = '${x} ${x}'\n"}});

  auto & root = parser->getRoot();
  EXPECT_EQ(requireField(root, "pure_float").kind(), hit::Field::Kind::Float);
  EXPECT_EQ(requireField(root, "pure_int").kind(), hit::Field::Kind::Int);
  EXPECT_EQ(requireField(root, "prefixed").kind(), hit::Field::Kind::String);
  EXPECT_EQ(requireField(root, "prefixed").strVal(), "foo_3");
  EXPECT_EQ(requireField(root, "two_exprs").kind(), hit::Field::Kind::String);
  EXPECT_EQ(requireField(root, "two_exprs").strVal(), "3 3");
}

TEST(ParserBraceExpansionTest, ExpandsNestedCommandArguments)
{
  auto parser = parseInputs({{"nested_args.i",
                              "n = 2\n"
                              "ranged = '${range 0 ${fparse 2 * n}}'\n"
                              "enumerated = '${enumerate block ${n} ${fparse n + 1}}'\n"}});

  auto & root = parser->getRoot();
  EXPECT_EQ(requireField(root, "ranged").strVal(), "0 1 2 3");
  EXPECT_EQ(requireField(root, "enumerated").strVal(), "block2 block3");
}

TEST(ParserBraceExpansionTest, ReportsCyclicBraceDependencyChain)
{
  // the walker reaches every field in the cycle, so each one reports the chain starting from itself
  const auto errors = parseErrors("a = ${b}\nb = ${c}\nc = ${a}\n");
  ASSERT_EQ(errors.size(), 3);
  EXPECT_NE(errors[0].message.find("cyclic brace-expression dependency detected: a -> b -> c -> a"),
            std::string::npos)
      << errors[0].message;
  for (const auto & error : errors)
    EXPECT_NE(error.message.find("cyclic brace-expression dependency detected"), std::string::npos)
        << error.message;
}

TEST(ParserBraceExpansionTest, ReportsEmptyBraceExpression)
{
  const auto errors = parseErrors("b = ${}\n");
  ASSERT_EQ(errors.size(), 1);
  expectErrorAt(errors[0], "empty brace expression in 'b'", 1, 5, 7);
}

TEST(ParserBraceExpansionTest, ReportsInvalidCommand)
{
  const auto errors = parseErrors("b = ${nosuchcmd 1 2}\n");
  ASSERT_EQ(errors.size(), 1);
  expectErrorAt(errors[0], "invalid brace-expression command 'nosuchcmd' in 'b'", 1, 5, 20);
}

TEST(ParserBraceExpansionTest, ReportsMissingSubstitutionVariable)
{
  const auto errors = parseErrors("b = foo_${missing}\n");
  ASSERT_EQ(errors.size(), 1);
  expectErrorAt(
      errors[0], "no variable 'missing' found for substitution expression in 'b'", 1, 9, 18);
}

TEST(ParserBraceExpansionTest, ReportsErrorSpanIndependentOfSpacing)
{
  {
    const auto errors = parseErrors("b=${nosuchcmd 1 2}\n");
    ASSERT_EQ(errors.size(), 1);
    expectErrorAt(errors[0], "invalid brace-expression command", 1, 3, 18);
  }
  {
    const auto errors = parseErrors("b    :=    ${nosuchcmd 1 2}\n");
    ASSERT_EQ(errors.size(), 1);
    expectErrorAt(errors[0], "invalid brace-expression command", 1, 12, 27);
  }
  {
    const auto errors = parseErrors("[s]\n  b = '  ${nosuchcmd 1 2}'\n[]\n");
    ASSERT_EQ(errors.size(), 1);
    expectErrorAt(errors[0], "invalid brace-expression command", 2, 10, 25);
  }
}

TEST(ParserBraceExpansionTest, ReportsErrorAtInnerExpression)
{
  // only the failing inner expression is reported, not the enclosing fparse that consumes it
  const auto errors = parseErrors("b = ${fparse ${units 1 m -> s} + 1}\n");
  ASSERT_EQ(errors.size(), 1);
  expectErrorAt(errors[0], "units error: m", 1, 14, 30);
}

TEST(ParserBraceExpansionTest, ReportsNonNumericFparseVariable)
{
  const auto errors = parseErrors("a = foo\nb = ${fparse a + 1}\n");
  ASSERT_EQ(errors.size(), 1);
  expectErrorAt(
      errors[0], "fparse error: variable 'a' with value 'foo' is not a number in 'b'", 2, 5, 19);
}
