//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "NonlinearSystemBase.h"

/**
 * Base class for storing and managing numerical data like solutions, residuals, and Jacobians.
 * The vectors are kept distributed with respect to the communicator of the application.
 * The whole snapshot vector is stored. The saving frequency can be defined using the `execute_on`
 * parameter.
 */
class SnapshotContainerBase : public GeneralReporter
{
public:
  static InputParameters validParams();
  SnapshotContainerBase(const InputParameters & parameters);

  /**
   * Storage for the snapshots.
   *
   * The underlying storage is unique_ptrs, but the public API
   * (read-only access) exposes just references.
   */
  class Snapshots : public UniqueStorage<NumericVector<Number>>
  {
  public:
    friend class SnapshotContainerBase;
  };

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

  /**
   * Return the whole snapshot container
   * @return const std::vector<std::unique_ptr<NumericVector<Number>>>&
   */
  const Snapshots & getSnapshots() const { return _accumulated_data; }

  /**
   * Return one of the stored snapshot vectors
   * @param local_i The index of the locally stored numeric data container
   */
  const NumericVector<Number> & getSnapshot(unsigned int local_i) const;

protected:
  /**
   * Clone the current snapshot vector.
   * @return std::unique_ptr<NumericVector<Number>>
   */
  virtual std::unique_ptr<NumericVector<Number>> collectSnapshot() = 0;

  /// Dynamic container for snapshot vectors. We store pointers to make sure that the change in size
  /// comes with little overhead. This is a reference because we need it to be restartable for
  /// stochastic runs in batch mode.
  Snapshots & _accumulated_data;

  /// The nonlinear system's number whose solution shall be collected
  const unsigned int _nonlinear_system_number;
};

void dataStore(std::ostream & stream, SnapshotContainerBase::Snapshots & v, void * context);
void dataLoad(std::istream & stream, SnapshotContainerBase::Snapshots & v, void * context);
