//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"
#include "Function.h"

/**
 * A specific BC for the mass (pressure) equation
 */
class INSADMassFreeBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  INSADMassFreeBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADVectorVariableValue & _velocity;

  const bool _has_vfn;

  const Function * _velocity_fn;

  /// The density
  const ADMaterialProperty<Real> & _rho;
};
