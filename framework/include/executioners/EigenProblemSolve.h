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

/**
 * EigenProblemSolve is used to solve an eigenvalue problem interfacing SLEPc.
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
