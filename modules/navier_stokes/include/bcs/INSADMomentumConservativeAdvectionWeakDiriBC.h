//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorIntegratedBC.h"

class Function;

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the advective term of the incompressible Navier-Stokes momentum
 * equation.
 */
class INSADMomentumConservativeAdvectionWeakDiriBC : public ADVectorIntegratedBC
{
public:
  static InputParameters validParams();

  INSADMomentumConservativeAdvectionWeakDiriBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The density
  const ADMaterialProperty<Real> & _rho;

  /// The Dirichlet value for velocity
  const Function & _diri_vel;
};
