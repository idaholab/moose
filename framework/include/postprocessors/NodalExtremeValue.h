//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalVariablePostprocessor.h"

// Input parameters
/// A postprocessor for collecting the nodal min or max value
class NodalExtremeValue : public NodalVariablePostprocessor
{
public:
  static InputParameters validParams();

  /// What type of extreme value we are going to compute
  enum ExtremeType
  {
    MAX,
    MIN
  };

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  NodalExtremeValue(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  /// The extreme value type ("min" or "max")
  ExtremeType _type;

  /**
   * The value of the variable at the point at which the proxy variable
   * reaches the max/min value.
   */
  Real _value;

  /**
   * A proxy variable used to find the quadrature point at
   * which to evaluate the variable. If not provided, defaults to the variable.
   */
  const VariableValue & _proxy_variable;

  /// Extreme value of the proxy variable
  Real _proxy_value;
};
