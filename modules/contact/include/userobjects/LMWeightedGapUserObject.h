//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedGapUserObject.h"

template <typename>
class MooseVariableFE;

/**
 * User object for computing weighted gaps and contact pressure for Lagrange multipler based
 * mortar constraints
 */
class LMWeightedGapUserObject : public WeightedGapUserObject
{
public:
  static InputParameters validParams();

  LMWeightedGapUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactPressure() const override;
  virtual void reinit() override {}

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return true; }

  /// The Lagrange multiplier variable representing the contact pressure
  const MooseVariableFE<Real> * const _lm_var;

  /// Whether to use Petrov-Galerkin approach
  const bool _use_petrov_galerkin;

  /// The auxiliary Lagrange multiplier variable (used together whith the Petrov-Galerkin approach)
  const MooseVariable * const _aux_lm_var;
};
