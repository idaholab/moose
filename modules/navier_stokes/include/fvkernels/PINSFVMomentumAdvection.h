//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableFV.h"
#include "INSFVMomentumAdvection.h"

/**
 * A flux kernel transporting momentum in porous media across cell faces
 */
class PINSFVMomentumAdvection : public INSFVMomentumAdvection
{
public:
  static InputParameters validParams();
  PINSFVMomentumAdvection(const InputParameters & params);

protected:
  /**
   * interpolation overload for the velocity
   */
  void interpolate(Moose::FV::InterpMethod m,
                   ADRealVectorValue & interp_v,
                   const ADRealVectorValue & elem_v,
                   const ADRealVectorValue & neighbor_v) override;

  virtual ADReal computeQpResidual() override;
  VectorValue<ADReal> coeffCalculator(const Elem & elem, const ADReal & mu) const override;

  /// porosity variable to compute gradients
  const MooseVariableFV<Real> * const _eps_var;
  /// porosity in the current element
  const VariableValue & _eps;
  /// porosity in the neighbor element
  const VariableValue & _eps_neighbor;
  /// Whether the porosity field is smooth or has discontinuities
  const bool _smooth_porosity;
};
