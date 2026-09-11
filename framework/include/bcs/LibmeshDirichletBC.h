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

/**
 * Imposes a constant essential boundary condition through libMesh's DOF constraint system
 */
class LibmeshDirichletBC : public LibmeshDirichletBCBase
{
public:
  static InputParameters validParams();

  LibmeshDirichletBC(const InputParameters & parameters);

  virtual Real value(const libMesh::Point & p, Real time) const override;

protected:
  /// The value for this BC
  const Real & _value;
};
