//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SidePostprocessor.h"

/**
 * This postprocessor computes displacements normal to a provided
 * set of boundaries
 */
class ObtainAvgContactAngle : public SidePostprocessor
{
public:
  static InputParameters validParams();

  ObtainAvgContactAngle(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  using Postprocessor::getValue;
  virtual Real getValue() const override;
  virtual void threadJoin(const UserObject & y) override;

protected:

  /// Gradient of coupled variable
  const VariableValue & _pf;

  /// Gradient of coupled variable
  const VariableGradient & _grad_pf;

  /// Average contact angle
  Real _contact_angle;

  /// cos theta value
  Real _cos_theta_val;

  /// total weight
  Real _total_weight;
};
