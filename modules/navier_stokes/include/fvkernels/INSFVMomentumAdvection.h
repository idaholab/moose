//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVAdvectionBase.h"

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics for the incompressible flow model
 */
class INSFVMomentumAdvection : public INSFVAdvectionBase
{
public:
  static InputParameters validParams();
  INSFVMomentumAdvection(const InputParameters & params);

protected:

  /**
   * method for computing the Rhie-Chow 'a' coefficients for the given elem \p elem
   * @param elem The elem to compute the Rhie-Chow coefficient for
   * @param mu The dynamic viscosity
   */
  VectorValue<ADReal> coeffCalculator(const Elem & elem, const ADReal & mu) const override;

  /// Density
  const Real & _rho;
};
