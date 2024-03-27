//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"
#include "libmesh/libmesh_config.h"
#include <petscsnes.h>

class NonlinearSystem;

/**
 * A problem that handles Schur complement preconditioning of the incompressible Navier-Stokes
 * equations
 */
class PrintMatricesNSProblem : public FEProblem
{
public:
  static InputParameters validParams();

  PrintMatricesNSProblem(const InputParameters & parameters);

protected:
  virtual void onTimestepEnd() override;

private:
  /// The tag name of the mass matrix
  const TagName & _mass_matrix;
  /// The tag name of the jump matrix
  const std::vector<TagName> & _jump_matrices;
};
