//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateTrainer.h"
#include "PolynomialQuadrature.h"
#include "QuadratureSampler.h"
#include "MultiDimPolynomialGenerator.h"

#include "Distribution.h"

class PolynomialChaosTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();
  PolynomialChaosTrainer(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

private:
  // TODO: Move as much of these to constructor initialization

  /// Sampler from which the parameters were perturbed
  Sampler * _sampler;

  /// QuadratureSampler pointer, necessary for applying quadrature weights
  QuadratureSampler * _quad_sampler;

  /// Vector postprocessor of the results from perturbing the model with _sampler
  const VectorPostprocessorValue * _values_ptr = nullptr;

  /// True when _sampler data is distributed
  bool _values_distributed;

  // The following items are stored using declareModelData for use as a trained model.

  /// Maximum polynomial order. The sum of 1D polynomial orders does not go above this value.
  const unsigned int & _order;

  /// Total number of parameters/dimensions
  unsigned int & _ndim;

  /// Total number of coefficient (defined by size of _tuple)
  std::size_t & _ncoeff;

  /// A _ndim-by-_ncoeff matrix containing the appropriate one-dimensional polynomial order
  std::vector<std::vector<unsigned int>> & _tuple;

  /// These are the coefficients we are after in the PC expansion
  std::vector<Real> & _coeff;

  /// The distributions used for sampling
  std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>> & _poly;
};
