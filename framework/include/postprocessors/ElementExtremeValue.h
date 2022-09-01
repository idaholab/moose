//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVariablePostprocessor.h"

// Input parameters
/// A postprocessor for collecting the elemental min or max value
class ElementExtremeValue : public ElementVariablePostprocessor
{
public:
  static InputParameters validParams();

  /// Type of extreme value we are going to compute
  enum ExtremeType
  {
    MAX,
    MIN
  };

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  ElementExtremeValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual Real getValue() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Get the extreme value at each quadrature point
  virtual void computeQpValue() override;

  /// The extreme value type
  ExtremeType _type;

  /**
   * A proxy variable used to find the quadrature point at
   * which to evaluate the variable. If not provided, defaults to the variable.
   */
  const VariableValue & _proxy_variable;

  /// Extreme value of the value and proxy variable at the same point
  std::pair<Real, Real> _proxy_value;
};
