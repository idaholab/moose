//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "libmesh/petsc_vector.h"

class OptimizationReporterBase;
/**
 * A UserObject that tests the requesting of Reporter values
 * that are actually correct.
 */
class OptimizationReporterTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  OptimizationReporterTest(const InputParameters & params);

  void initialSetup() override;
  void initialize() override{};
  void execute() override;
  void finalize() override{};

private:
  const Real _tol;
  /// Communicator used for operations
  const libMesh::Parallel::Communicator _my_comm;
  OptimizationReporterBase * _optReporter = nullptr;
  std::unique_ptr<libMesh::PetscVector<Number>> _optSolverParameters;
};
