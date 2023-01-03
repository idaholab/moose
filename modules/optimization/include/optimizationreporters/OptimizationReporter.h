//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationData.h"
#include "DataIO.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
// friends
#include "OptimizeSolve.h"
#include "OptimizationReporterTest.h"

/**
 * Computes objective function, gradient and contains reporters for communicating between
 * optimizeSolve and subapps
 */
class OptimizationReporter : public OptimizationData
{
public:
  static InputParameters validParams();
  OptimizationReporter(const InputParameters & parameters);

  void initialize() override final {}
  void execute() override final {}
  void finalize() override final {}

  /**
   * Function to initialize petsc vectors from vpp data
   * FIXME: this should be const
   */
  void setInitialCondition(libMesh::PetscVector<Number> & param);

  /**
   * Function to override misfit values with the simulated values from the matrix free hessian
   * forward solve
   */
  void setMisfitToSimulatedValues();

  /**
   * Functions to check if bounds are set
   */
  bool hasBounds() const { return _upper_bounds.size() > 0 && _lower_bounds.size() > 0; }

  /**
   * Upper and lower bounds for each parameter being controlled
   * @return vector containing one entry per controllable parameter for each upper/lower bound
   */
  const std::vector<Real> & getUpperBounds() const { return _upper_bounds; };
  const std::vector<Real> & getLowerBounds() const { return _lower_bounds; };

  /**
   * Function to compute objective.
   * This is the last function called in objective routine
   */
  virtual Real computeObjective();

  /**
   * Function to compute gradient.
   * This is the last call of the gradient routine.
   */
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) const;

  /**
   * Function to get the total number of parameters
   * @return total number of parameters
   */
  unsigned int getNumParams() const { return _ndof; };

protected:
  /// Parameter names
  const std::vector<ReporterValueName> & _parameter_names;
  /// Number of parameter vectors
  const unsigned int _nparam;
  /// Number of values for each parameter
  const std::vector<dof_id_type> & _nvalues;
  /// Total number of parameters
  const dof_id_type _ndof;

  /// Parameter values declared as reporter data
  std::vector<std::vector<Real> *> _parameters;

  /// Bounds of the parameters
  const std::vector<Real> & _lower_bounds;
  const std::vector<Real> & _upper_bounds;

  /// vector of adjoint data
  const std::vector<Real> & _adjoint_data;

  /**
   * Function to set parameters.
   * This is the first function called in objective/gradient/hessian routine
   */
  void updateParameters(const libMesh::PetscVector<Number> & x);

private:
  friend class OptimizeSolve;
  friend class OptimizationReporterTest;

  void setSimulationValuesForTesting(std::vector<Real> & data);
};
