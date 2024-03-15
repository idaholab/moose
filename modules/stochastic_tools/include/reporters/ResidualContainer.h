//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "NonlinearSystemBase.h"
#include "SnapshotContainerBase.h"
#include "SystemBase.h"
#include "libmesh/petsc_vector.h"

/**
 * This class is responsible for collecting residual vectors in one place. The
 * vectors are kept distributed with respect to the communicator of the application.
 * The whole residual vector is stored.
 * The saving frequency can be defined using the `execute_on` parameter.
 */
class ResidualContainer : public SnapshotContainerBase
{
public:
  static InputParameters validParams();
  ResidualContainer(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<NumericVector<Number>> collectSnapshot() override;

  const NonlinearSystem & _nl_sys;
  const TagID _tag_id;
};
