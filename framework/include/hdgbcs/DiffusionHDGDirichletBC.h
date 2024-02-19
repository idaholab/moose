//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGIntegratedBC.h"
#include "DiffusionHDGInterface.h"

#include <vector>

class Function;

/**
 * Weakly imposes Dirichlet boundary conditions for a hybridized discretization of diffusion
 */
class DiffusionHDGDirichletBC : public HDGIntegratedBC, public DiffusionHDGInterface
{
public:
  static InputParameters validParams();

  DiffusionHDGDirichletBC(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onBoundary() override;

private:
  /// Dirichlet function
  const Function & _dirichlet_val;

  friend class DiffusionHDGInterface;
};
