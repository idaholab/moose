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

  /// Get the reduced solution for a given parameter sample.
  void evaluateSolution(const std::vector<Real> & params);

  /// Get the reduced solution for a given parameter sample and reconstruct the
  /// approximate solution into a given vector.
  void evaluateSolution(const std::vector<Real> & params,
                        DenseVector<Real> & inp_vector,
                        std::string var_name);

  /// Get a reference to the approximate solutions.
  const std::vector<DenseVector<Real>> & getApproximateSolution() const { return _approx_solution; }

  /// Get the nodal QoI of the reconstructed solution for a given variable.
  Real getNodalQoI(std::string var_name, unsigned int qoi_type) const;

protected:
  /// Initialize reduced matrices, vectors and additional containers.
  void initializeReducedSystem();

  /// Initialize approximate solution vector.
  void initializeApproximateSolution();

  /// Assemble and solve the reduced equation system.
  void solveReducedSystem(const std::vector<Real> & params);

  /// Reconstruct the approximate solution vector using the stored
  /// coefficients.
  void reconstructApproximateSolution();

  /// Reconstruct the approximate solution vector into an input vector.
  void reconstructApproximateSolution(DenseVector<Real> & inp_vector, std::string var_name);

  /// A vector containing the number of basis functions each variable should use.
  /// This is optional, used only to override the base numbers from the training data.
  std::vector<std::string> _change_rank;

  /// The new rank the variable should have.
  std::vector<unsigned int> _new_ranks;

  /// The final rank that should be used for every variable.
  std::vector<unsigned int> _final_ranks;

  /// Comulative ranks of the system. Used for indexing only.
  std::vector<unsigned int> _comulative_ranks;

  /// Vector containing the names of the variables we want to reconstruct.
  const std::vector<std::string> & _var_names;

  /// Strings describing which operator is indepedent of the solution and which
  /// corresponds to a reduced DirichletBC operator.
  const std::vector<std::string> & _tag_types;

  /// The basis vectors for all the variables.
  const std::vector<std::vector<DenseVector<Real>>> & _base;

  /// The reduced operators in the same order as given in tag_types.
  const std::vector<DenseMatrix<Real>> & _red_operators;

  /// Coefficients of the reduced order model.
  DenseVector<Real> _coeffs;

  /// The reduced system matrix.
  DenseMatrix<Real> _sys_mx;

  /// The reduced right hand side.
  DenseVector<Real> _rhs;

  /// Reconstructed solution for each variable
  std::vector<DenseVector<Real>> _approx_solution;

  /// Penalty parameter for Dirichlet BCs.
  const Real _penalty;

  /// Switch that is set to see if the ROM matrices and vectors are initialized.
  bool _initialized;
};
