//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

/**
 * Computes the volume-weighted L1 or L2 norm of the change of a variable over a
 * time step or between nonlinear iterations.
 */
class AverageVariableChange : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  AverageVariableChange(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;
  virtual Real getValue() const override;
  virtual Real computeQpIntegral() override;

protected:
  /// Interval over which to compute change
  const MooseEnum & _change_over;
  /// "Old" solution value in the change
  const VariableValue & _u_change_old;

  /// Selected norm
  const MooseEnum & _norm;
  /// Norm exponent
  const unsigned int _norm_exponent;

  /// Subdomain(s) volume
  Real _volume;
};
