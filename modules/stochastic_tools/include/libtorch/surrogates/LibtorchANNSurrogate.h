//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include "LibtorchArtificialNeuralNet.h"
#include "SurrogateModel.h"
#include "Standardizer.h"

class LibtorchANNSurrogate : public SurrogateModel
{
public:
  static InputParameters validParams();
  LibtorchANNSurrogate(const InputParameters & parameters);

  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const override;

protected:
  /// Pointer to the neural net object (initialized as null)
  const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & _nn;

  /// Standardizer for use with input (x)
  const StochasticTools::Standardizer & _input_standardizer;

  /// Standardizer for use with output response (y)
  const StochasticTools::Standardizer & _output_standardizer;
};

#endif
