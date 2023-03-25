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
   * Functions to check if bounds are set
   */
  bool hasBounds() const { return _upper_bounds.size() && _lower_bounds.size(); }

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

  /**
   * Helper function to get i,j index into the parameter vector of vector for
   * an index into a flattened form of this parameter vector used by petsc
   *
   * @param index The DoF index in the flattened parameter vector
   * @return std::pair<size_t,size_t> The i,j index into the parameter vector of vector
   */
  virtual std::pair<std::size_t, std::size_t> getParameterIndex(dof_id_type index) const;

  /**
   * Fill lower and upper bounds vector of vector member data for each parameter in each group
   */
  void fillBounds();

  /**
   * Fill initial conditions vector of vector member data for each parameter in each group
   */
  void fillInitialConditions();

  /**
   * Initialize parameter and gradient reporters
   */
  void initializeOptimizationReporters();

  /// Parameter names
  const std::vector<ReporterValueName> & _parameter_names;
  /// Number of parameter vectors
  const unsigned int _nparams;

  /// Parameter values declared as reporter data
  std::vector<std::vector<Real> *> _parameters;
  /// Gradient values declared as reporter data
  std::vector<std::vector<Real> *> _gradients;

  /// Bounds of the parameters
  std::vector<std::vector<Real>> _lower_bounds;
  std::vector<std::vector<Real>> _upper_bounds;

  /// initial conditions of the parameters
  std::vector<std::vector<Real>> _initial_conditions;

  /// Number of values for each parameter group
  std::vector<dof_id_type> _nvalues;
  /// Total number of parameters
  dof_id_type _ndof;

private:
  friend class OptimizeSolve;
  friend class OptimizationReporterTest;

  /**
   * helper fill method for vector of vectors of parameters in each parameter group
   * Called by fillBounds and fillInitialConditions
   * @param type the param type to read
   * @return std::vector<std::vector<Real>> value for each parameter in each group
   */
  std::vector<std::vector<Real>> fillVectorOfVectors(std::string type) const;

  /**
   * helper to fill fillBounds and fillInitialConditions with default values
   * @param defaultValue value to fill with
   * @param data member vector of vector to fill
   */
  void fillWithDefaults(Real defaultValue, std::vector<std::vector<Real>> & data) const;

  void setSimulationValuesForTesting(std::vector<Real> & data);
};
