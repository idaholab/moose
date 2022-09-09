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
#include "ExtremeValueBase.h"

/// A postprocessor for collecting the elemental min or max value
class ElementExtremeValue : public ExtremeValueBase<ElementVariablePostprocessor>
{
public:
  static InputParameters validParams();

  ElementExtremeValue(const InputParameters & parameters);

protected:
  virtual std::pair<Real, Real> getProxyValuePair() override;

  virtual void computeQpValue() override { computeExtremeValue(); }

  /**
   * A proxy variable used to find the quadrature point at
   * which to evaluate the variable. If not provided, defaults to the variable.
   */
  const VariableValue & _proxy_variable;

  /// Extreme value of the value and proxy variable at the same point
  std::pair<Real, Real> _proxy_value;
};
