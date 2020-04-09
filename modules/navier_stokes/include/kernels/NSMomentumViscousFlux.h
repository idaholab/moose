//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSKernel.h"
#include "NSViscStressTensorDerivs.h"

// ForwardDeclarations

/**
 * Derived instance of the NSViscousFluxBase class
 * for the momentum equations.
 */
class NSMomentumViscousFlux : public NSKernel
{
public:
  static InputParameters validParams();

  NSMomentumViscousFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Required parameter
  const unsigned int _component;

  // An object for computing viscous stress tensor derivatives.
  // Constructed via a reference to ourself
  NSViscStressTensorDerivs<NSMomentumViscousFlux> _vst_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSViscStressTensorDerivs;
};
