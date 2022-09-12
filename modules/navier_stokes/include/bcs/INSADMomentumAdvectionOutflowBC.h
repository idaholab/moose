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
#include "MooseEnum.h"

// Forward Declarations

/**
 * This class implements the "No BC" boundary condition based on the
 * "Laplace" form of the viscous stress tensor.
 */
class INSADMomentumAdvectionOutflowBC : public ADVectorIntegratedBC
{
public:
  static InputParameters validParams();

  INSADMomentumAdvectionOutflowBC(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;
  const ADMaterialProperty<Real> & _rho;
  const VariableValue & _eps;
};
