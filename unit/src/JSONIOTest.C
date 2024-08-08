//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "JsonIO.h"
#include "MooseTypes.h"

#include "libmesh/int_range.h"
#include "libmesh/dense_matrix.h"

TEST(JSONIOTest, libMeshDenseMatrix)
{
  const std::size_t m = 2;
  const std::size_t n = 3;
  std::vector<std::vector<Real>> values(m, std::vector<Real>(n));
  libMesh::DenseMatrix<Real> matrix(m, n);

  Real current_value = 1;
  for (const auto i : make_range(m))
    for (const auto j : make_range(n))
    {
      values[i][j] = current_value;
      matrix(i, j) = current_value;
      current_value += 2;
    }

  nlohmann::json json;
  nlohmann::to_json(json, matrix);
  const std::vector<std::vector<Real>> json_values = json;
  EXPECT_EQ(json_values, values);
}

// This is guarded because there are issues with early implementations of
// filesystem in the standard library that we have with our minimum
// GCC and clang compilers
#if __GLIBCXX__ > 20210514 || (!defined(__clang__) && defined(NDEBUG))
template <typename T>
void
testDerivativeStringClass()
{
  const T value("some_" + (std::string) typeid(T).name());
  nlohmann::json json;
  nlohmann::to_json(json, value);
  const std::string json_value = json;
  EXPECT_EQ(json_value, static_cast<std::string>(value));
}

TEST(JSONIOTest, derivativeStringClasses)
{
  testDerivativeStringClass<FileName>();
  testDerivativeStringClass<FileNameNoExtension>();
  testDerivativeStringClass<MeshFileName>();
  testDerivativeStringClass<MatrixFileName>();
  testDerivativeStringClass<OutFileBase>();
  testDerivativeStringClass<NonlinearVariableName>();
  testDerivativeStringClass<AuxVariableName>();
  testDerivativeStringClass<VariableName>();
  testDerivativeStringClass<BoundaryName>();
  testDerivativeStringClass<SubdomainName>();
  testDerivativeStringClass<PostprocessorName>();
  testDerivativeStringClass<VectorPostprocessorName>();
  testDerivativeStringClass<MeshDivisionName>();
  testDerivativeStringClass<FunctionName>();
  testDerivativeStringClass<DistributionName>();
  testDerivativeStringClass<SamplerName>();
  testDerivativeStringClass<UserObjectName>();
  testDerivativeStringClass<IndicatorName>();
  testDerivativeStringClass<MarkerName>();
  testDerivativeStringClass<MultiAppName>();
  testDerivativeStringClass<OutputName>();
  testDerivativeStringClass<MaterialPropertyName>();
  testDerivativeStringClass<MooseFunctorName>();
  testDerivativeStringClass<MaterialName>();
  testDerivativeStringClass<TagName>();
  testDerivativeStringClass<MeshGeneratorName>();
  testDerivativeStringClass<ExtraElementIDName>();
  testDerivativeStringClass<ReporterValueName>();
  testDerivativeStringClass<PositionsName>();
  testDerivativeStringClass<TimesName>();
  testDerivativeStringClass<ExecutorName>();
  testDerivativeStringClass<ParsedFunctionExpression>();
  testDerivativeStringClass<NonlinearSystemName>();
  testDerivativeStringClass<CLIArgString>();
}
#endif

TEST(JSONIOTest, uniquePtrSerializer)
{
  {
    auto value = std::make_unique<unsigned int>(5);
    nlohmann::json json = value;
    const unsigned int json_value = json;
    EXPECT_EQ(json_value, *value);
  }

  {
    std::unique_ptr<unsigned int> null_value;
    nlohmann::json json = null_value;
    EXPECT_TRUE(json.is_null());
  }
}
