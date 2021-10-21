//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * This object adds a residual equivalent to
 *
 * \f$\int_{\Omega_C} -\epsilon \frac{p}{r} dV\f$
 *
 * for use when performing axisymmetric simulations and the \f$\epsilon \nabla p\f$ term has been
 * integrated by parts as is done for both HLLC and Kurganov-Tadmor schemes
 */
class PINSFVMomentumPressureFluxRZ : public FVElementalKernel
{
public:
  static InputParameters validParams();
  PINSFVMomentumPressureFluxRZ(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const VariableValue & _p;
  const VariableValue & _eps;
};
