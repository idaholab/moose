//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
#include "Calculators.h"

#include "libmesh/dense_matrix.h"

namespace StochasticTools
{
/**
 * Calculator for computing Sobol sensitivity indices according to the paper by Saltelli (2002)
 * https://doi.org/10.1016/S0010-4655(02)00280-1
 *
 * The data provided is stacked vectors provided by the SobolSampler. Example use of this object
 * is also available in the stochastic_tools unit testing.
 */
class SobolCalculator : public Calculator<std::vector<std::vector<Real>>, std::vector<Real>>
{
public:
  SobolCalculator(const libMesh::ParallelObject & other,
                  const std::string & name,
                  bool resample);

protected:
  virtual void initialize() override;
  virtual void update(const std::vector<Real> & data) override;
  virtual void finalize(bool is_distributed) override;
  virtual std::vector<Real> get() const override { return _sobol; }

private:
  /// Set to true if the resampling matrix exists for computing second-order indices
  const bool _resample;
  /// Matrix containing dot products of data
  DenseMatrix<Real> _amat;
  /// The returned sobol indices
  std::vector<Real> _sobol;
};
} // namespace
