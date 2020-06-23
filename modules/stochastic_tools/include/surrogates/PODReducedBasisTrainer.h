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
#include "MultiApp.h"
#include "MooseTypes.h"

class PODReducedBasisTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  PODReducedBasisTrainer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override;

  virtual void execute() override;

  virtual void finalize() override;

  void initReducedOperators();

  void addSnapshot(unsigned int v_ind, DenseVector<Real>& snapshot);

  void addToReducedOperator(unsigned int base_i, unsigned int tag_i, std::vector<DenseVector<Real>>& residual);

  const std::vector<std::string>& getVarNames() const
  {
    return _var_names;
  }

  const std::vector<std::string>& getTagNames() const
  {
    return _tag_names;
  }

  unsigned int getBaseSize(unsigned int v_ind)
  {
    return _base[v_ind].size();
  }

  const DenseVector<Real>& getBasisVector(unsigned int v_index, unsigned int base_i) const
  {
    return _base[v_index][base_i];
  }

  unsigned int getSumBaseSize();

protected:

  void computeEigenDecomposition();

  void computeCorrelationMatrix();

  void computeBasisVectors();

  void printReducedOperators();

  std::vector<std::string> _var_names;

  std::vector<std::string> _tag_names;

  /// The snapshots contained for this problem
  std::vector<std::vector<DenseVector<Real>>> _snapshots;

  /// The reduced operators which will be assembled
  std::vector<DenseMatrix<Real>> _red_operators;

  /// The reduced basis for the variables
  std::vector<std::vector<DenseVector<Real>>> _base;

  /// The correlation matrices for the variables
  std::vector<DenseMatrix<Real>> _corr_mx;

  /// Vector containing the eigenvvalues of the correlation matrix
  std::vector<DenseVector<Real>> _eigenvalues;

  /// Matirx containing the eigenvectors of the correlation matrix
  std::vector<DenseMatrix<Real>> _eigenvectors;

  bool _base_completed;

private:

  /// True when _sampler data is distributed
  // bool _values_distributed = false; // default to false; set in initialSetup
};
