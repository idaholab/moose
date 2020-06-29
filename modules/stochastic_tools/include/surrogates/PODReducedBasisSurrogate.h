//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateModel.h"

class PODReducedBasisSurrogate : public SurrogateModel
{
public:
  static InputParameters validParams();

  PODReducedBasisSurrogate(const InputParameters & parameters);

  void initializeReducedSystem();

  void evaluateSolution(const std::vector<Real> & params);

  void solveReducedSystem(const std::vector<Real> & params);

  void reconstructApproximateSolution();

  std::vector<DenseVector<Real>>& getApproximateSolution(){return _approx_solution;}

  virtual Real evaluate(const std::vector<Real> & x) const override;

  Real getMax(std::string var_name) const;

protected:
  /// Coefficients of the reduced order model.
  DenseVector<Real> _coeffs;

  /// The reduced system matrix.
  DenseMatrix<Real> _sys_mx;

  /// The reduced right hand side.
  DenseVector<Real> _rhs;

  /// Reconstructed solution for each variable
  std::vector<DenseVector<Real>> _approx_solution;

  /// Vector containing the names of the variables we want to reconstruct.
  const std::vector<std::string>& _var_names;

  /// Bools describing which operator is indepedent of the solution.
  const std::vector<unsigned int>& _independent;

  /// The basis vectors for all the variables.
  const std::vector<std::vector<DenseVector<Real>>>& _base;

  /// The power matrix for the terms in the polynomial expressions.
  const std::vector<DenseMatrix<Real>>& _red_operators;

  /// Switch that is set to see if the ROM matrices and vectors are initialized.
  bool _initialized;

private:

};
