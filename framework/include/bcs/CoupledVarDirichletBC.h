//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBCBase.h"

/**
 * Implements a Dirichlet BC where u=factor * _coupled_var on the boundary.
 */
class CoupledVarDirichletBC : public DirichletBCBase
{
public:
  static InputParameters validParams();

  CoupledVarDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;
  virtual Real computeQpJacobian(unsigned int jvar);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Variable providing the value u on the boundary.
  const VariableValue & _coupled_var;

  /// The identifying number of the coupled variable
  const unsigned int _coupled_num;
  
  /// Scale factor
  const Function & _scale_factor;
};
