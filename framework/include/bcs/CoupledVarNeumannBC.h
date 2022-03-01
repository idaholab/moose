//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Implements a Neumann BC where grad(u)=_coupled_var on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class CoupledVarNeumannBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  CoupledVarNeumannBC(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;
  Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Variable providing the value of grad(u) on the boundary.
  const VariableValue & _coupled_var;

  /// The identifying number of the coupled variable
  const unsigned int _coupled_num;

  /// A coefficient that is multiplied with the residual contribution
  const Real _coef;

  /// Scale factor
  const VariableValue & _scale_factor;
};
