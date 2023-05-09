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

// friends
#include "OptimizeSolve.h"
#include "OptimizationReporterTest.h"

namespace libMesh
{
template <typename Number>
class PetscVector;
}

/**
 * Base class for optimization objects, implements routines for calculating misfit. Derived classes
 * are responsible for parameter members and gradient computation.
 */
class OptimizationReporterBase : public OptimizationData
{
public:
  static InputParameters validParams();
  OptimizationReporterBase(const InputParameters & parameters);

  void initialize() override final {}
  void execute() override final {}
  void finalize() override final {}

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
   * Function to initialize petsc vectors from vpp data
   */
  void setInitialCondition(libMesh::PetscVector<Number> & param);

  /**
   * Function to override misfit values with the simulated values from the matrix free hessian
   * forward solve
   */
  void setMisfitToSimulatedValues();

  /**
   * Upper and lower bounds for each parameter being controlled
   *
   * @param i Parameter index
   * @return The upper/lower bound for parameter i
   */
  Real getUpperBound(dof_id_type i) const;
  Real getLowerBound(dof_id_type i) const;

  /**
   * Function to get the total number of parameters
   * @return total number of parameters
   */
  dof_id_type getNumParams() const { return _ndof; }

protected:
  /**
   * Function to set parameters.
   * This is the first function called in objective/gradient/hessian routine
   */
  virtual void updateParameters(const libMesh::PetscVector<Number> & x);

  /// Parameter names
  const std::vector<ReporterValueName> & _parameter_names;
  /// Number of parameter vectors
  const unsigned int _nparams;

  /// Parameter values declared as reporter data
  std::vector<std::vector<Real> *> _parameters;
  /// Gradient values declared as reporter data
  std::vector<std::vector<Real> *> _gradients;

  /// Bounds of the parameters
  std::vector<Real> _lower_bounds;
  std::vector<Real> _upper_bounds;

  /// Number of values for each parameter
  std::vector<dof_id_type> _nvalues;
  /// Total number of parameters
  dof_id_type _ndof;

private:
  friend class OptimizeSolve;
  friend class OptimizationReporterTest;

  void setSimulationValuesForTesting(std::vector<Real> & data);
};
