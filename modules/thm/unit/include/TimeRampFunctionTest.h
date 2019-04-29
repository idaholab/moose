#pragma once

#include "MooseObjectUnitTest.h"

class TimeRampFunctionTest : public MooseObjectUnitTest
{
public:
  TimeRampFunctionTest()
    : MooseObjectUnitTest("THMTestApp"),

      _initial_value(1.0),
      _final_value(2.0),
      _ramp_duration(6.0),
      _initial_time(2.0),

      _fn_name("tested_function")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    // tested function
    {
      const std::string class_name = "TimeRampFunction";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("initial_value") = _initial_value;
      params.set<Real>("final_value") = _final_value;
      params.set<Real>("ramp_duration") = _ramp_duration;
      params.set<Real>("initial_time") = _initial_time;
      _fe_problem->addFunction(class_name, _fn_name, params);
    }
  }

  const Real _initial_value;
  const Real _final_value;
  const Real _ramp_duration;
  const Real _initial_time;
  const FunctionName _fn_name;
};
