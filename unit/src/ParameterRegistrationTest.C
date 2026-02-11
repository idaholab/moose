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

template <typename T>
void
testValue(const std::string & hit_value, const T & value = T())
{
  const auto test_field = buildTestField(hit_value);
  auto test_param = buildTestParam<T>();

  Moose::ParameterRegistry::get().set(*test_param.param, *test_field.field);

  ASSERT_EQ(*test_param.value, value);
}

template <typename T, class ExceptionType = std::invalid_argument>
void
testValueError(const std::string & hit_value, const std::string & error)
{
  EXPECT_THROW_MSG(testValue<T>(hit_value), ExceptionType, error);
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
    EXPECT_THROW_MSG(Moose::ParameterRegistration::setScalarValue(value, *test_field.field),
                     std::invalid_argument,
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
    EXPECT_THROW_MSG(Moose::ParameterRegistration::setVectorValue(value, *test_field.field),
                     std::invalid_argument,
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
    EXPECT_THROW_MSG(Moose::ParameterRegistration::setDoubleVectorValue(value, *test_field.field),
                     std::invalid_argument,
                     "invalid syntax for int double vector parameter: " + param_name +
                         "[1]='a b c'");
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

  // empty vector early return
  {
    const auto test_field = buildTestField("\"\"");
    std::vector<std::vector<std::vector<int>>> value;
    Moose::ParameterRegistration::setTripleVectorValue(value, *test_field.field);
    ASSERT_EQ(value, std::vector<std::vector<std::vector<int>>>{});
  }

  // failed to convert
  {
    const auto test_field = buildTestField("\"1 | 2; 3 a \"");
    std::vector<std::vector<std::vector<int>>> value;
    EXPECT_THROW_MSG(Moose::ParameterRegistration::setTripleVectorValue(value, *test_field.field),
                     std::invalid_argument,
                     "invalid syntax for int triple vector parameter: " + param_name +
                         "[1][1]='3 a'");
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
    EXPECT_THROW_MSG(
        Moose::ParameterRegistration::setMapValue(value, *test_field.field),
        std::invalid_argument,
        "odd number of entries for map parameter '" + param_name +
            "'; there must be an even number or else you will end up with a key without a value");
  }

  // key conversion failed
  {
    const auto test_field = buildTestField("\"a 1\"");
    std::map<unsigned int, double> value;
    EXPECT_THROW_MSG(Moose::ParameterRegistration::setMapValue(value, *test_field.field),
                     std::invalid_argument,
                     "invalid unsigned int syntax for map parameter '" + param_name + "' key: 'a'");
  }

  // value conversion failed
  {
    const auto test_field = buildTestField("\"1 a\"");
    std::map<unsigned int, double> value;
    EXPECT_THROW_MSG(Moose::ParameterRegistration::setMapValue(value, *test_field.field),
                     std::invalid_argument,
                     "invalid double syntax for map parameter '" + param_name + "' value: 'a'");
  }

  // duplicate keys
  {
    const auto test_field = buildTestField("\"1 2 1 2\"");
    std::map<unsigned int, double> value;
    EXPECT_THROW_MSG(Moose::ParameterRegistration::setMapValue(value, *test_field.field),
                     std::invalid_argument,
                     "duplicate entry for map parameter: '" + param_name +
                         "'; key '1' appears multiple times");
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

template <typename T>
void
testNumericScalar()
{
  testValue<T>("1", T(1));

  // signed, test -1
  if constexpr (std::is_signed_v<T>)
  {
    testValue<T>("-1", T(-1));
  }
  // unsigned, negatives not allowed
  else
  {
    testValueError<T>("-1",
                      "invalid syntax for " + MooseUtils::prettyCppType<T>() +
                          " parameter: " + param_name + "='-1'");
  }

  // floating point values
  if constexpr (std::is_floating_point_v<T>)
  {
    testValue<T>("10.000000", T(10));
    testValue<T>("1.234", T(1.234));
    testValue<T>("1.23e-4", T(1.23e-4));
    testValue<T>("-2.34e-2", T(-2.34e-2));
  }
}

TEST(ParameterRegistrationTest, testNumericScalars)
{
  testNumericScalar<double>();
  testNumericScalar<short int>();
  testNumericScalar<int>();
  testNumericScalar<long int>();
  testNumericScalar<unsigned short>();
  testNumericScalar<unsigned int>();
  testNumericScalar<unsigned long>();
  testNumericScalar<unsigned long long>();
}

TEST(ParameterRegistrationTest, testBool)
{
  const std::vector<std::string> trues{
      "true", "TRUE", "trUe", "on", "ON", "oN", "yes", "YES", "yEs", "1"};
  const std::vector<std::string> falses{
      "false", "FALSE", "faLse", "off", "OFF", "oFF", "no", "NO", "nO", "0"};

  // successful scalar trues, with and without quotes
  for (const auto & v : trues)
  {
    testValue<bool>(v, true);
    testValue<bool>("\"" + v + "\"", true);
  }
  // successful scalar falses, with and without quotes
  for (const auto & v : falses)
  {
    testValue<bool>(v, false);
    testValue<bool>("\"" + v + "\"", false);
  }
  // failed scalar
  testValueError<bool>("foo", "invalid boolean syntax for parameter: " + param_name + "='foo'");

  // vector bools
  for (const auto & true_v : trues)
    for (const auto & false_v : falses)
    {
      // successful mixes
      testValue<std::vector<bool>>("\"" + true_v + " " + false_v + " \"", {true, false});
      testValue<std::vector<bool>>("\"" + false_v + " " + true_v + " \"", {false, true});
      // failed value
      testValueError<std::vector<bool>>(
          "\"true bar\"", "invalid syntax for bool vector parameter: " + param_name + "[1]='bar'");
    }
}

TEST(ParameterRegistrationTest, testScalarComponentValue)
{
  // success
  testValue<Point>("\"1 2 3.0\"", Point(1, 2, 3));
  testValue<RealVectorValue>("\"4 5.0 6\"", RealVectorValue(4, 5, 6));

  // must be size 3
  testValueError<Point>("\"1\"",
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
  testValueError<RealEigenVector, hit::Error>(
      "a", "file:1.1: cannot convert field 'key' value 'a' to float");
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
  testValueError<RealEigenMatrix>("\"3; 1 2\"", "matrix is not square for parameter key");

  // invalid syntax
  testValueError<RealEigenMatrix>("\"1 2.0; a 1\"",
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
  testValueError<RealTensorValue>(
      "\"1\"", "invalid RealTensorValue parameter '" + param_name + "': size is 1 but should be 9");

  // conversion failed
  testValueError<RealTensorValue, hit::Error>(
      "a", "file:1.1: cannot convert field 'key' value 'a' to float");
}

TEST(ParameterRegistrationTest, testReporterName)
{
  // success
  testValue<ReporterName>("foo/bar", {"foo", "bar"});

  // can't split without /
  testValueError<ReporterName>("foo",
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
  testValueError<std::vector<Point>>("1",
                                     "wrong number of values in vector parameter '" + param_name +
                                         "': size 1 is not a multiple of 3");

  // conversion failed
  testValueError<std::vector<Point>, hit::Error>(
      "a", "file:1.1: cannot convert field 'key' value 'a' to float");
}

TEST(ParameterRegistrationTest, testVectorMooseEnum)
{
  const auto test_field = buildTestField("\"a b\"");
  auto test_param = buildTestParam<std::vector<MooseEnum>>();
  *test_param.value = {{"a b c"}};
  Moose::ParameterRegistry::get().set(*test_param.param, *test_field.field);
  ASSERT_EQ(test_param.value->size(), 2);
  ASSERT_TRUE((*test_param.value)[0] == "a");
  ASSERT_TRUE((*test_param.value)[1] == "b");
}

TEST(ParameterRegistrationTest, testVectorMultiMooseEnum)
{
  // Success
  {
    const auto test_field = buildTestField("\"a b; a\"");
    auto test_param = buildTestParam<std::vector<MultiMooseEnum>>();
    *test_param.value = {{"a b c"}};
    Moose::ParameterRegistry::get().set(*test_param.param, *test_field.field);
    ASSERT_EQ(test_param.value->size(), 2);
    ASSERT_TRUE((*test_param.value)[0].isValueSet("a"));
    ASSERT_TRUE((*test_param.value)[0].isValueSet("b"));
    ASSERT_FALSE((*test_param.value)[0].isValueSet("c"));
    ASSERT_TRUE((*test_param.value)[1].isValueSet("a"));
    ASSERT_FALSE((*test_param.value)[1].isValueSet("b"));
    ASSERT_FALSE((*test_param.value)[1].isValueSet("c"));
  }
}

// Still need to test:
// - vector<CLIArgString>

TEST(ParameterRegistrationTest, testVectorReporterName)
{
  // success
  testValue<std::vector<ReporterName>>("\"foo/bar baz/bang\"", {{"foo", "bar"}, {"baz", "bang"}});

  // can't split without /
  testValueError<std::vector<ReporterName>>(
      "\"foo/bar baz\"",
      "invalid syntax in ReporterName parameter " + param_name +
          ": supplied name 'baz' must contain the '/' delimiter");
}

TEST(ParameterRegistrationTest, testDoubleVectorComponentValue)
{
  // success
  testValue<std::vector<std::vector<Point>>>("\"1 2 3.0\n4.0 5 6; 7 8 9.0\"",
                                             {{{1, 2, 3}, {4, 5, 6}}, {{7, 8, 9}}});
  testValue<std::vector<std::vector<RealVectorValue>>>("\"1 2 3; 4.0 5 6\n7 8.0 9\"",
                                                       {{{1, 2, 3}}, {{4, 5, 6}, {7, 8, 9}}});

  // bad size
  testValueError<std::vector<std::vector<Point>>>(
      "\"1 2 3; 4 5 6.0\n7.0 8\"",
      "wrong number of values in double-indexed vector component parameter '" + param_name +
          "' at index 1: subcomponent size 5 is not a multiple of 3");

  // conversion failed
  testValueError<std::vector<std::vector<Point>>>(
      "a", "invalid format for parameter '" + param_name + "' at index 0");
}
