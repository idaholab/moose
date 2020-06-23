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

  void addSnapshot(std::string var_name, DenseVector<Real>& snapshot);

  const std::vector<std::string>& varNames() const
  {
    return _var_names;
  }

  unsigned int getBaseSize(std::string var_name)
  {
    return _base[var_name].size();
  }

  const DenseVector<Real>& getBasisVector(std::string var_name, unsigned int base_i) const
  {
    return _base.at(var_name)[base_i];
  }

  unsigned int getSumBaseSize();

protected:

  void computeEigenDecomposition();

  void computeCorrelationMatrix();

  void computeBasisVectors();

  std::vector<std::string> _var_names;

  /// The snapshots contained for this problem
  std::map<std::string, std::vector<DenseVector<Real>>> _snapshots;

  /// The reduced basis for the variables
  std::map<std::string, std::vector<DenseVector<Real>>> _base;

  /// The correlation matrices for the variables
  std::map<std::string, DenseMatrix<Real>> _corr_mx;

  /// Vector containing the eigenvvalues of the correlation matrix
  std::map<std::string, DenseVector<Real>> _eigenvalues;

  /// Matirx containing the eigenvectors of the correlation matrix
  std::map<std::string, DenseMatrix<Real>> _eigenvectors;

private:

  /// True when _sampler data is distributed
  // bool _values_distributed = false; // default to false; set in initialSetup
};
