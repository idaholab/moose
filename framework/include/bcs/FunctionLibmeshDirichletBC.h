//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LibmeshDirichletBCBase.h"

class Function;

/**
 * Imposes an essential boundary condition given by a MOOSE Function through libMesh's DOF
 * constraint system
 */
class FunctionLibmeshDirichletBC : public LibmeshDirichletBCBase
{
public:
  static InputParameters validParams();

  FunctionLibmeshDirichletBC(const InputParameters & parameters);

  virtual Real value(const libMesh::Point & p, Real time) const override;

protected:
  /// The function being used for evaluation
  const Function & _func;
};
