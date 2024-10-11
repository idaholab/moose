//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SegregatedSolverBase.h"
#include "RhieChowMassFlux.h"
#include "PetscSupport.h"
#include "SolverParams.h"
#include "SIMPLESolve.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/equation_systems.h"
#include "libmesh/solver_configuration.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Executioner set up to solve a thermal-hydraulics problem using the SIMPLE algorithm.
 * It utilizes segregated linear systems which are solved using a fixed-point iteration.
 */
class SIMPLE : public SegregatedSolverBase
{
public:
  static InputParameters validParams();

  SIMPLE(const InputParameters & parameters);

  virtual void init() override;
  virtual void execute() override;
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  SIMPLESolve _simple_solve;
};
