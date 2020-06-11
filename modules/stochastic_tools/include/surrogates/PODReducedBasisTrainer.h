//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"

class PODReducedBasisTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  PODReducedBasisTrainer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override;

  virtual void execute() override;

  virtual void finalize() override;

  void addSnapshot(NumericVector<Number>& new_vector);

protected:
  /// The snapshots contained for this problem
  std::vector<std::unique_ptr<NumericVector<Number>>> _solution_vectors;

  /// The reduced basis for the variables
  std::vector<std::unique_ptr<NumericVector<Number>>> _base;

  /// The correlation matrices for the variables
  DenseMatrix<Real> _corr_mx;

  /// Vector containing the eigenvvalues of the correlation matrix
  DenseVector<Real> _eigenvalues;

  /// Matirx containing the eigenvectors of the correlation matrix
  DenseMatrix<Real> _eigenvectors;

private:

  unsigned int _no_snaps;

  /// True when _sampler data is distributed
  // bool _values_distributed = false; // default to false; set in initialSetup
};
