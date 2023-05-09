//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedVelocitiesUserObject.h"

template <typename>
class MooseVariableFE;

/**
 * Nodal-based mortar contact user object for frictional problem
 */
class LMWeightedVelocitiesUserObject : public WeightedVelocitiesUserObject
{
public:
  static InputParameters validParams();

  LMWeightedVelocitiesUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactTangentialPressureDirOne() const override;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const override;

  virtual const ADVariableValue & contactPressure() const override;

  virtual void reinit() override {}

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return true; }

  /// The Lagrange multiplier variables representing the contact pressure along various directions
  const MooseVariableFE<Real> * const _lm_normal_var;
  const MooseVariableFE<Real> * const _lm_variable_tangential_one;
  const MooseVariableFE<Real> * const _lm_variable_tangential_two;

  /// Whether to use Petrov-Galerkin approach
  const bool _use_petrov_galerkin;

  /// The auxiliary Lagrange multiplier variable (used together whith the Petrov-Galerkin approach)
  const MooseVariable * const _aux_lm_var;
};
