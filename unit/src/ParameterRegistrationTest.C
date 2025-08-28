//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "hit/hit.h"

#include "ParameterRegistry.h"
#include "ParameterRegistration.h"

#include "MooseUnitUtils.h"

#include "ReporterName.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"

const std::string param_name = "key";

struct TestField
{
  std::unique_ptr<const hit::Node> root = nullptr;
  const hit::Field * field = nullptr;
};

TestField
buildTestField(const std::string & hit_value)
{
  TestField test_field;
  test_field.root.reset(hit::parse("file", param_name + " = " + hit_value));
  test_field.field = dynamic_cast<const hit::Field *>(test_field.root->find(param_name));
  return test_field;
}

template <typename T>
struct TestParam
{
  libMesh::Parameters params;
  libMesh::Parameters::Value * param;
  T * value;
};

template <typename T>
TestParam<T>
buildTestParam()
{
  TestParam<T> test_param;
  test_param.value = &test_param.params.template set<T>(param_name);
  test_param.param = test_param.params.begin()->second.get();
  return test_param;
}

template <typename T, bool near = false>
void
testValue(const std::string & hit_value, const T & value = T())
{
  const auto test_field = buildTestField(hit_value);
  auto test_param = buildTestParam<T>();

  Moose::ParameterRegistry::get().set(*test_param.param, *test_field.field);

  if constexpr (near)
    ASSERT_NEAR(*test_param.value, value, 1e-12);
  else
    ASSERT_EQ(*test_param.value, value);
}

TEST(ParameterRegistrationTest, setScalarValue)
{
  // success
  {
    const auto test_field = buildTestField("100");
    int value;
    Moose::ParameterRegistration::setScalarValue(value, *test_field.field);
    ASSERT_EQ(value, 100);
  }

  // failure to convert, throws
  {
    const std::string hit_value = "a";
    const auto test_field = buildTestField(hit_value);
    int value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setScalarValue(value, *test_field.field); },
        "invalid syntax for int parameter: " + param_name + "='" + hit_value + "'");
  }
}

TEST(ParameterRegistrationTest, setVectorValue)
{
  // success
  {
    const auto test_field = buildTestField("\"1 2   \n3 4\n5    6\"");
    std::vector<int> value;
    Moose::ParameterRegistration::setVectorValue(value, *test_field.field);
    const std::vector<int> expected_value{1, 2, 3, 4, 5, 6};
    ASSERT_EQ(value, expected_value);
  }

  // failure to convert, throws
  {
    const std::string hit_value = "\"1 a \"";
    const auto test_field = buildTestField(hit_value);
    std::vector<int> value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setVectorValue(value, *test_field.field); },
        "invalid syntax for int vector parameter: " + param_name + "[1]='a'");
  }
}

TEST(ParameterRegistrationTest, setDoubleVectorValue)
{
  // success
  {
    const auto test_field = buildTestField("\"1 2\n   3 4; 5\n  6 7  8\"");
    std::vector<std::vector<int>> value;
    Moose::ParameterRegistration::setDoubleVectorValue(value, *test_field.field);
    const std::vector<std::vector<int>> expected_value{{1, 2, 3, 4}, {5, 6, 7, 8}};
    ASSERT_EQ(value, expected_value);
  }

  // empty string early return
  {
    const auto test_field = buildTestField("\" \n \n\"");
    std::vector<std::vector<int>> value;
    Moose::ParameterRegistration::setDoubleVectorValue(value, *test_field.field);
    ASSERT_TRUE(value.empty());
  }

  // failure to convert, throws
  {
    const std::string hit_value = "\"1; a b c   \"";
    const auto test_field = buildTestField(hit_value);
    std::vector<std::vector<int>> value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setDoubleVectorValue(value, *test_field.field); },
        "invalid syntax for int double vector parameter: " + param_name + "[1]='a b c'");
  }
}

TEST(ParameterRegistrationTest, setTripleVectorValue)
{
  // success
  {
    const auto test_field = buildTestField("\" 1 2 3;\n    4 5 6; | 7; 8;\n9\"");
    std::vector<std::vector<std::vector<int>>> value;
    Moose::ParameterRegistration::setTripleVectorValue(value, *test_field.field);
    const std::vector<std::vector<std::vector<int>>> expected_value{{{1, 2, 3}, {4, 5, 6}, {}},
                                                                    {{7}, {8}, {9}}};
    ASSERT_EQ(value, expected_value);
  }

  // failed to convert
  {
    const auto test_field = buildTestField("\"1 | 2; 3 a \"");
    std::vector<std::vector<std::vector<int>>> value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setTripleVectorValue(value, *test_field.field); },
        "invalid syntax for int triple vector parameter: " + param_name + "[1][1]='3 a'");
  }
}

TEST(ParameterRegistrationTest, setMapValue)
{
  // success
  {
    const auto test_field = buildTestField("\"1 2\n3  \n4\"");
    std::map<unsigned int, double> value;
    Moose::ParameterRegistration::setMapValue(value, *test_field.field);
    const std::map<unsigned int, double> expected_value{{1, 2}, {3, 4}};
    ASSERT_EQ(value, expected_value);
  }

  // odd entries
  {
    const auto test_field = buildTestField("1");
    std::map<unsigned int, double> value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setMapValue(value, *test_field.field); },
        "odd number of entries for map parameter '" + param_name + "'");
  }

  // key conversion failed
  {
    const auto test_field = buildTestField("\"a 1\"");
    std::map<unsigned int, double> value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setMapValue(value, *test_field.field); },
        "invalid unsigned int syntax for map parameter '" + param_name + "' key: 'a'");
  }

  // value conversion failed
  {
    const auto test_field = buildTestField("\"1 a\"");
    std::map<unsigned int, double> value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setMapValue(value, *test_field.field); },
        "invalid double syntax for map parameter '" + param_name + "' value: 'a'");
  }

  // duplicate keys
  {
    const auto test_field = buildTestField("\"1 2 1 2\"");
    std::map<unsigned int, double> value;
    Moose::UnitUtils::assertThrows<std::invalid_argument>(
        [&value, &test_field]()
        { Moose::ParameterRegistration::setMapValue(value, *test_field.field); },
        "duplicate entry for map parameter: '" + param_name + "'; key '1' appears multiple times");
  }
}

TEST(ParameterRegistrationTest, testDerivativeString)
{
  testValue<FileName>("foo", "foo");
  testValue<std::vector<FileName>>("\"foo bar\"", {"foo", "bar"});
  testValue<std::vector<std::vector<FileName>>>("\"foo bar; baz\"", {{"foo", "bar"}, {"baz"}});
  testValue<std::vector<std::vector<std::vector<FileName>>>>(
      "\"foo bar; baz | bang\"", {{{"foo", "bar"}, {"baz"}}, {{"bang"}}});
}

TEST(ParameterRegistrationTest, testNativeScalars)
{
  // double
  testValue<double>("1", 1);
  testValue<double>("-1", -1);
  testValue<double>("10.000000", 10);
  testValue<double>("1.234", 1.234);
  testValue<double>("1.2e-4", 1.2e-4);
  testValue<double>("-10e-2", -10e-2);
}

TEST(ParameterRegistrationTest, testBool)
{
  const std::vector<std::string> trues{
      "true", "TRUE", "trUe", "on", "ON", "oN", "yes", "YES", "yEs", "1"};
  const std::vector<std::string> falses{
      "false", "FALSE", "faLse", "off", "OFF", "oFF", "no", "NO", "nO", "0"};

  for (const auto & v : trues)
  {
    testValue<bool>(v, true);
    testValue<bool>("\"" + v + "\"", true);
  }
  for (const auto & v : falses)
  {
    testValue<bool>(v, false);
    testValue<bool>("\"" + v + "\"", false);
  }

  for (const auto & true_v : trues)
    for (const auto & false_v : falses)
    {
      testValue<std::vector<bool>>("\"" + true_v + " " + false_v + " \"", {true, false});
      testValue<std::vector<bool>>("\"" + false_v + " " + true_v + " \"", {false, true});
    }
}

TEST(ParameterRegistrationTest, testScalarComponentValue)
{
  // success
  testValue<Point>("\"1 2 3.0\"", Point(1, 2, 3));
  testValue<RealVectorValue>("\"4 5.0 6\"", RealVectorValue(4, 5, 6));

  // must be size 3
  Moose::UnitUtils::assertThrows<std::invalid_argument>(
      []() { testValue<Point>("\"1\""); },
      "wrong number of values in libMesh::Point parameter '" + param_name +
          "': was given 1 component(s) but should have 3");
}

TEST(ParameterRegistrationTest, testRealEigenVector)
{
  // success
  RealEigenVector expected_value(2);
  expected_value(0) = 1;
  expected_value(1) = 2;
  testValue<RealEigenVector>("\" 1\n2.0\"", expected_value);

  // conversion failed
  Moose::UnitUtils::assertThrows<hit::Error>([]() { testValue<RealEigenVector>("a"); },
                                             "cannot convert field 'key' value 'a' to float");
}

TEST(ParameterRegistrationTest, testRealEigenMatrix)
{
  // success
  RealEigenMatrix expected_value(2, 2);
  expected_value(0, 0) = 1;
  expected_value(0, 1) = 2;
  expected_value(1, 0) = 3;
  expected_value(1, 1) = 4;
  testValue<RealEigenMatrix>("\"1.0 2; 3 4.0\"", expected_value);

  // not square
  Moose::UnitUtils::assertThrows<std::invalid_argument>(
      []() { testValue<RealEigenMatrix>("\"3; 1 2\""); }, "matrix is not square");

  // invalid syntax
  Moose::UnitUtils::assertThrows<std::invalid_argument>(
      []() { testValue<RealEigenMatrix>("\"1 2.0; a 1\""); },
      "invalid syntax for parameter: " + param_name + "[1]='a 1'");
}

TEST(ParameterRegistrationTest, testMooseEnum)
{
  const auto test_field = buildTestField("a");
  auto test_param = buildTestParam<MooseEnum>();
  *test_param.value = MooseEnum("a b");
  Moose::ParameterRegistry::get().set(*test_param.param, *test_field.field);
  ASSERT_TRUE(*test_param.value == "a");
  ASSERT_FALSE(*test_param.value == "b");
}

TEST(ParameterRegistrationTest, testMultiMooseEnum)
{
  const auto test_field = buildTestField("\"a c\"");
  auto test_param = buildTestParam<MultiMooseEnum>();
  *test_param.value = MultiMooseEnum("a b c");
  Moose::ParameterRegistry::get().set(*test_param.param, *test_field.field);
  ASSERT_TRUE(test_param.value->isValueSet("a"));
  ASSERT_FALSE(test_param.value->isValueSet("b"));
  ASSERT_TRUE(test_param.value->isValueSet("c"));
}

TEST(ParameterRegistrationTest, testExecFlagEnum)
{
  const auto test_field = buildTestField("\"custom final\"");
  auto test_param = buildTestParam<ExecFlagEnum>();
  *test_param.value = MooseUtils::getDefaultExecFlagEnum();
  *test_param.value = {};
  Moose::ParameterRegistry::get().set(*test_param.param, *test_field.field);
  ASSERT_TRUE(test_param.value->isValueSet("custom"));
  ASSERT_TRUE(test_param.value->isValueSet("final"));
}

TEST(ParameterRegistrationTest, testRealTensorValue)
{
  // success
  testValue<RealTensorValue>("\" 1 2 3 4.0 5\n6 7 8.0 9\"",
                             RealTensorValue(1, 2, 3, 4, 5, 6, 7, 8, 9));

  // must have 9 components
  Moose::UnitUtils::assertThrows<std::invalid_argument>(
      []() { testValue<RealTensorValue>("\"1\""); },
      "invalid RealTensorValue parameter '" + param_name + "': size is 1 but should be 9");

  // conversion failed
  Moose::UnitUtils::assertThrows<hit::Error>([]() { testValue<RealTensorValue>("a"); },
                                             "cannot convert field 'key' value 'a' to float");
}

TEST(ParameterRegistrationTest, testReporterName)
{
  // success
  testValue<ReporterName>("foo/bar", {"foo", "bar"});

  // can't split without /
  Moose::UnitUtils::assertThrows<std::invalid_argument>(
      []() { testValue<ReporterName>("foo"); },
      "invalid syntax in ReporterName parameter " + param_name +
          ": supplied name 'foo' must contain the '/' delimiter");
}

TEST(ParameterRegistrationTest, testVectorComponentValue)
{
  // success
  testValue<std::vector<Point>>("\"1 2 3.0\n4.0 5 6\"", std::vector<Point>{{1, 2, 3}, {4, 5, 6}});
  testValue<std::vector<RealVectorValue>>("\"2 3.0 4\n5 6.0 7\"",
                                          std::vector<RealVectorValue>{{2, 3, 4}, {5, 6, 7}});

  // bad size
  Moose::UnitUtils::assertThrows<std::invalid_argument>(
      []() { testValue<std::vector<Point>>("1"); },
      "wrong number of values in vector parameter '" + param_name +
          "': size 1 is not a multiple of 3");

  // conversion failed
  Moose::UnitUtils::assertThrows<hit::Error>([]() { testValue<std::vector<Point>>("a"); },
                                             "cannot convert field 'key' value 'a' to float");
}

// Still need to test:
// - vector<MooseEnum>
// - vector<MultiMooseEnum>
// - vector<CLIArgString>
// double component value

TEST(ParameterRegistrationTest, testVectorReporterName)
{
  // success
  testValue<std::vector<ReporterName>>("\"foo/bar baz/bang\"", {{"foo", "bar"}, {"baz", "bang"}});

  // can't split without /
  Moose::UnitUtils::assertThrows<std::invalid_argument>(
      []() { testValue<std::vector<ReporterName>>("\"foo/bar baz\""); },
      "invalid syntax in ReporterName parameter " + param_name +
          ": supplied name 'baz' must contain the '/' delimiter");
}
