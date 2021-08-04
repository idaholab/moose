//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "StochasticReporter.h"
#include "SurrogateModelInterface.h"
#include "SurrogateModel.h"

/**
 * A tool for output Sampler data.
 */
class EvaluateSurrogate : public StochasticReporter, SurrogateModelInterface
{
public:
  static InputParameters validParams();

  EvaluateSurrogate(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// Sampler for evaluating surrogate model
  Sampler & _sampler;
  /// The data type for the response value
  const MultiMooseEnum _response_types;
  /// Whether or not to compute standard deviation
  std::vector<bool> _doing_std;
  /// Pointers to surrogate model
  std::vector<const SurrogateModel *> _model;
  ///@{
  /// Vectors containing results of sampling model
  std::vector<std::vector<Real> *> _real_values;
  std::vector<std::vector<std::vector<Real>> *> _vector_real_values;
  std::vector<std::vector<Real> *> _real_std;
  std::vector<std::vector<std::vector<Real>> *> _vector_real_std;
  ///@}
};
