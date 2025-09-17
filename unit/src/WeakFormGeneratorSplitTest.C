#include "WeakFormGenerator.h"
#include "VariableSplitting.h"
#include "MooseAST.h"

#include "gtest/gtest.h"

using namespace moose::automatic_weak_form;

namespace
{

std::map<std::string, NodePtr>
extractDefinitions(const std::map<std::string, SplitVariable> & split_vars)
{
  std::map<std::string, NodePtr> defs;
  for (const auto & [name, sv] : split_vars)
    defs[name] = sv.definition;
  return defs;
}

} // namespace

TEST(WeakFormGeneratorSplitTest, LaplacianEnergyMatchesBaseline)
{
  VariableSplittingAnalyzer analyzer(1, 3);

  NodePtr u = fieldVariable("u");
  NodePtr energy = multiply(laplacian(u), laplacian(u));

  WeakFormGenerator baseline_generator(3);
  NodePtr baseline_strong = baseline_generator.generateWeakForm(energy, "u");
  ASSERT_TRUE(baseline_strong);

  auto split_vars = analyzer.generateSplitVariables(energy);
  auto split_defs = extractDefinitions(split_vars);
  NodePtr transformed = analyzer.transformExpression(energy, split_vars);

  WeakFormGenerator split_generator(3);
  split_generator.setSplitDefinitions(split_defs);
  NodePtr split_strong = split_generator.generateWeakForm(transformed, "u");
  ASSERT_TRUE(split_strong);

  EXPECT_EQ(split_strong->toString(), baseline_strong->toString());
}

TEST(WeakFormGeneratorSplitTest, SplitVariableCarriesDerivativeInformation)
{
  VariableSplittingAnalyzer analyzer(1, 3);

  NodePtr u = fieldVariable("u");
  NodePtr expr = laplacian(u);

  auto split_vars = analyzer.generateSplitVariables(expr);
  auto split_defs = extractDefinitions(split_vars);

  ASSERT_TRUE(split_vars.count("u_d2"));
  auto split_it = split_vars.find("u_d2");

  DifferentiationVisitor dv("u", &split_defs);
  auto split_diff = dv.differentiate(fieldVariable(split_it->first));
  auto reference_expr = div(grad(fieldVariable("u")));
  auto reference_diff = dv.differentiate(reference_expr);

  EXPECT_EQ(split_diff.maxOrder(), reference_diff.maxOrder());
  for (unsigned int order = 0; order <= reference_diff.maxOrder(); ++order)
  {
    EXPECT_EQ(split_diff.hasOrder(order), reference_diff.hasOrder(order));
    if (reference_diff.hasOrder(order))
    {
      auto split_coeff = split_diff.getCoefficient(order);
      auto ref_coeff = reference_diff.getCoefficient(order);
      ASSERT_NE(split_coeff, nullptr);
      ASSERT_NE(ref_coeff, nullptr);
      EXPECT_EQ(split_coeff->toString(), ref_coeff->toString());
    }
  }
}
