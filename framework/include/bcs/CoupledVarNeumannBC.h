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

class CoupledVarNeumannBC;

template <>
InputParameters validParams<CoupledVarNeumannBC>();

/**
 * Implements a Neumann BC where grad(u)=_coupled_var on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class CoupledVarNeumannBC : public IntegratedBC
{
public:
  CoupledVarNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Variable providing the value of grad(u) on the boundary.
  const VariableValue & _coupled_var;
};
