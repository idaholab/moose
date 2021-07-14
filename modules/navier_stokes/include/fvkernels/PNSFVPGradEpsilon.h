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

class Function;

/**
 * This object adds a residual equivalent to
 *
 * \f$\int_{\Omega_C} -p \nabla \epsilon dV\f$
 *
 * This object must be included in any simulations where the \$\epsilon \nabla p\f$
 * term has been integrated by parts, as is done for Kurganov-Tadmor and
 * HLLC, and when the porosity has spatial dependence
 */
class PNSFVPGradEpsilon : public FVElementalKernel
{
public:
  static InputParameters validParams();
  PNSFVPGradEpsilon(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// The pressure
  const ADMaterialProperty<Real> & _pressure;

  /// The porosity as a function
  const Function & _eps_function;

  /// index x|y|z
  const unsigned int _index;
};
