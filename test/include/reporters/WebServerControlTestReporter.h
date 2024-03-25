//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

#include <optional>

/*
 * Object that tests setting controllable values for the WebServerControl
 */
class WebServerControlTestReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  WebServerControlTestReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  struct TestValueBase
  {
    virtual ~TestValueBase() = default;
    virtual void update() = 0;
  };

  template <typename T>
  struct TestValue : public TestValueBase
  {
    TestValue(T & reporter_value, const T & param_value)
      : reporter_value(&reporter_value), param_value(&param_value)
    {
    }
    virtual void update() override final { *reporter_value = *param_value; }

  private:
    T * reporter_value;
    const T * param_value;
  };

  template <typename T>
  void declare(const std::string & param_name)
  {
    if (isParamValid(param_name))
      _values.emplace_back(std::make_unique<TestValue<T>>(
          declareValueByName<T>(param_name, REPORTER_MODE_DISTRIBUTED), getParam<T>(param_name)));
  }

  std::vector<std::unique_ptr<TestValueBase>> _values;
};
