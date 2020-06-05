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
#include "SurrogateTrainer.h"

/**
 * TODO: Inherit from MooseObject, add to the TheWarehouse, and create [ObjectiveFunctions] block
 *
 * This also should support distributed data and probably operate on PETSc dense matrix/vectors
 */
class ObjectiveFunction : public GeneralUserObject
{
public:
  static InputParameters validParams();
  ObjectiveFunction(const InputParameters & parameters);

  // Removed, GUO is just being used to test out the concept
  virtual void initialize() final {}
  virtual void finalize() final {}
  virtual void execute() final {}

  // Used for evaluation, need to create ObjectFunctionModel that uses training data created
  // by the GradientDescentTraing
  virtual Real value(const DenseVector<Real> & x) const = 0;
  virtual DenseVector<Real> gradient(const DenseVector<Real> & x) const = 0;
  virtual unsigned int size() const = 0;
};

class PolynomialLeastSquares : public ObjectiveFunction
{
public:
  static InputParameters validParams();
  PolynomialLeastSquares(const InputParameters & parameters);

  virtual Real value(const DenseVector<Real> & x) const override;
  virtual DenseVector<Real> gradient(const DenseVector<Real> & x) const override;
  virtual unsigned int size() const override;

protected:
  const unsigned int _order;
  const VectorPostprocessorValue & _x_training_data;
  const VectorPostprocessorValue & _y_training_data;

private:
  // TODO: Fix order of operations so things to not need to be setup in gradient function
  mutable DenseVector<Real> _b_vector;
  mutable DenseMatrix<Real> _A_matrix;
  mutable DenseVector<Real> _Ax_minus_b;
};

class Polynomial
{
public:
  Polynomial() = default;

  DenseMatrix<Real> buildMatrix(const std::vector<Real> & x_training_data);

protected:
  const unsigned int _order = 1;
};

// Start of Optimizer? Need to include ways to plugin custom step-size and regularize
// Perhaps we create a new block
class GradientDescentTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();
  GradientDescentTrainer(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  const ObjectiveFunction & _objective_function;

  const unsigned int _max_iterations;
  const Real _step_size;

  DenseVector<Real> & _function_values;
};
