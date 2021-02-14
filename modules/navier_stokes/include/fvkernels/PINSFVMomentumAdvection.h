//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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

  /**
   * Returns the Rhie-Chow 'a' coefficient for the requested element \p elem
   * @param elem The elem to get the Rhie-Chow coefficient for
   * @param mu The dynamic viscosity
   */
  const VectorValue<ADReal> & rcCoeff(const Elem & elem, const ADReal & mu) const override;

  /// porosity
  const VariableValue & _eps;
  const VariableValue & _eps_neighbor;
};
