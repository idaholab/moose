//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#include "Executioner.h"

class InputParameters;
class EigenProblem;

template <typename T>
InputParameters validParams();

/**
 * EigenProblemSolve is used to drive the eigenvalue calculations. At the end,
 * SLEPc will be involved.
 * We derive from Executioner instead of Steady because 1) we want to have a fine-grain
 * control such as recovering; 2) Conceptually, Steady is very different from Eigenvalue,
 * where the former handles a nonlinear system of equations while the later targets
 * at an eigenvalue problem.
 */
class EigenProblemSolve : public FEProblemSolve
{
public:
  static InputParameters validParams();

  EigenProblemSolve(Executioner & ex);

  virtual void initialSetup() override;

protected:
  EigenProblem & _eigen_problem;

  /// Postprocessor value that scales solution when eigensolve is finished
  const PostprocessorValue * const _normalization;
};
