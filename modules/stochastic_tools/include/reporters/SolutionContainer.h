//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SnapshotContainerBase.h"
#include "UniqueStorage.h"

#include "libmesh/petsc_vector.h"

/**
 * This class is responsible for collecting solution vectors in one place. The
 * vectors are kept distributed with respect to the communicator of the application.
 * The whole solution vector is stored, which may contain multiple variables.
 * The saving frequency can be defined using the `execute_on` parameter.
 */
class SolutionContainer : public SnapshotContainerBase
{
public:
  static InputParameters validParams();
  SolutionContainer(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<NumericVector<Number>> collectSnapshot() override;
  /// Enum to switch between collecting the solution vectors of auxiliary and nonlinear
  /// systems
  const MooseEnum _system_type;
};
