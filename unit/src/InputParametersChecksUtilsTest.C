//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObjectUnitTest.h"
#include "InputParametersChecksUtils.h"

/// Minimal object that mixes in InputParametersChecksUtils so its protected checks
/// can be exercised. It declares a few optional parameters that are considered
/// "set by the user" only when their value is provided to the factory.
class CheckParamsTestObject : public MooseObject,
                              public InputParametersChecksUtils<CheckParamsTestObject>
{
public:
  static InputParameters validParams();

  CheckParamsTestObject(const InputParameters & params)
    : MooseObject(params), InputParametersChecksUtils<CheckParamsTestObject>(this)
  {
  }

  /// Public wrapper around the protected check under test
  void checkAtMostOne(const std::vector<std::string> & params) const
  {
    checkAtMostOneParamSetByUser(params);
  }
};

registerMooseObject("MooseUnitApp", CheckParamsTestObject);

InputParameters
CheckParamsTestObject::validParams()
{
  auto params = MooseObject::validParams();
  params.registerBase("CheckParamsTestObject");
  params.addParam<Real>("A", "Optional parameter A");
  params.addParam<Real>("B", "Optional parameter B");
  params.addParam<Real>("C", "Optional parameter C");
  return params;
}

class InputParametersChecksUtilsTest : public MooseObjectUnitTest
{
public:
  InputParametersChecksUtilsTest() : MooseObjectUnitTest("MooseUnitApp") {}

protected:
  /// Build a CheckParamsTestObject in which every parameter in \p set_params has
  /// been set by the user (any other declared parameter is left unset).
  std::shared_ptr<CheckParamsTestObject> buildObject(const std::vector<std::string> & set_params)
  {
    InputParameters params = _factory.getValidParams("CheckParamsTestObject");
    params.set<SubProblem *>("_subproblem") = _fe_problem.get();
    for (const auto & param : set_params)
      params.set<Real>(param) = 1.0;
    return _factory.create<CheckParamsTestObject>(
        "CheckParamsTestObject", "test_object_" + std::to_string(_count++), params, 0);
  }

  unsigned int _count = 0;
};

// No parameter set by the user: the check passes.
TEST_F(InputParametersChecksUtilsTest, noneSet)
{
  auto obj = buildObject({});
  EXPECT_NO_THROW(obj->checkAtMostOne({"A", "B", "C"}));
}

// Exactly one parameter set by the user: the check passes.
TEST_F(InputParametersChecksUtilsTest, oneSet)
{
  auto obj = buildObject({"B"});
  EXPECT_NO_THROW(obj->checkAtMostOne({"A", "B", "C"}));
}

// Two of the checked parameters set by the user: the check errors and reports
// the offending parameters.
TEST_F(InputParametersChecksUtilsTest, twoSet)
{
  auto obj = buildObject({"A", "C"});
  try
  {
    obj->checkAtMostOne({"A", "B", "C"});
    FAIL() << "expected checkAtMostOneParamSetByUser to error";
  }
  catch (const std::exception & e)
  {
    const std::string msg(e.what());
    EXPECT_NE(msg.find("Only one parameter of"), std::string::npos) << msg;
    EXPECT_NE(msg.find("should be set but"), std::string::npos) << msg;
    // The two parameters that were set should both be reported
    EXPECT_NE(msg.find("A"), std::string::npos) << msg;
    EXPECT_NE(msg.find("C"), std::string::npos) << msg;
  }
}

// All checked parameters set by the user: the check errors.
TEST_F(InputParametersChecksUtilsTest, allSet)
{
  auto obj = buildObject({"A", "B", "C"});
  EXPECT_THROW(obj->checkAtMostOne({"A", "B", "C"}), std::exception);
}

// Only parameters in the provided list are considered: setting a parameter that
// is not listed does not trigger the error.
TEST_F(InputParametersChecksUtilsTest, onlyCountsListedParams)
{
  auto obj = buildObject({"A", "C"});
  EXPECT_NO_THROW(obj->checkAtMostOne({"A", "B"}));
}
