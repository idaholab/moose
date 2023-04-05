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
   * Function to initialize petsc vectors from vpp data
   */
  virtual void setInitialCondition(libMesh::PetscVector<Number> & param) = 0;

  virtual void setInitialCondition(std::vector<int> & ix, std::vector<Real> & rx) {}

  /**
   * Function to override misfit values with the simulated values from the matrix free hessian
   * forward solve
   */
  void setMisfitToSimulatedValues();

  /**
   * Functions to check if bounds are set
   */
  virtual bool hasBounds() const = 0;

  /**
   * Upper and lower bounds for each parameter being controlled
   *
   * @param i Parameter index
   * @return The upper/lower bound for parameter i
   */
  virtual Real getUpperBound(dof_id_type i) const;
  virtual Real getLowerBound(dof_id_type i) const;

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
  virtual dof_id_type getNumParams() const = 0;

  virtual void updateParameters(const std::vector<int> & ix, const std::vector<Real> & rx) {}
protected:
  /**
   * Function to set parameters.
   * This is the first function called in objective/gradient/hessian routine
   */
  virtual void updateParameters(const libMesh::PetscVector<Number> & x) = 0;


private:
  friend class OptimizeSolve;
  friend class OptimizationReporterTest;

  void setSimulationValuesForTesting(std::vector<Real> & data);
};
