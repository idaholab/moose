//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

class SobolSampler;

template <>
InputParameters validParams<SobolSampler>();
/**
 * A class used to perform Monte Carlo Sampling
 */
class SobolSampler : public Sampler
{
public:
  SobolSampler(const InputParameters & parameters);

protected:
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;
  virtual void sampleSetUp() override;
  virtual void sampleTearDown() override;

  ///@{
  /// Sobol Monte Carlo matrices, these are sized and cleared to avoid keeping large matrices around
  DenseMatrix<Real> _a_matrix;
  DenseMatrix<Real> _b_matrix;
  ///@}

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  /// The number of rows per matrix
  const dof_id_type _num_rows_per_matrix;
};
