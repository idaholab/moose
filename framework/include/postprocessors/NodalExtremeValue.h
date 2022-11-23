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
#include "ExtremeValueBase.h"

/// A postprocessor for collecting the nodal min or max value
class NodalExtremeValue : public ExtremeValueBase<NodalVariablePostprocessor>
{
public:
  static InputParameters validParams();

  NodalExtremeValue(const InputParameters & parameters);
  virtual void execute() override { computeExtremeValue(); }

protected:
  virtual std::pair<Real, Real> getProxyValuePair() override;

  /**
   * A proxy variable used to find the quadrature point at
   * which to evaluate the variable. If not provided, defaults to the variable.
   */
  const VariableValue & _proxy_variable;

  /// Extreme value of the proxy variable
  Real _proxy_value;
};
