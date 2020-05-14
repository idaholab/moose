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
class INSADMomentumNoBCBC : public ADVectorIntegratedBC
{
public:
  static InputParameters validParams();

  INSADMomentumNoBCBC(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADVariableValue & _p;
  const bool _integrate_p_by_parts;
  const ADMaterialProperty<Real> & _mu;

  /// The form of the viscous term. Either laplace or traction
  MooseEnum _form;
};
