//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef LIBTORCH_ENABLED
#include <torch/torch.h>
#include "LibtorchArtificialNeuralNet.h"
#endif

#include "SurrogateModel.h"

class LibtorchANNSurrogate : public SurrogateModel
{
public:
  static InputParameters validParams();
  LibtorchANNSurrogate(const InputParameters & parameters);

  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const override;

protected:
#ifdef LIBTORCH_ENABLED
  /// Pointer to the neural net object (initialized as null)
  const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & _nn;
#endif
};
