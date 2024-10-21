//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once
#include <torch/types.h>
#include "GeneralVectorPostprocessor.h"

/**
 * A UserObject that tests the LibtorchArtificialNeuralNet.
 */
class LibtorchArtificialNeuralNetTest : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  LibtorchArtificialNeuralNetTest(const InputParameters & params);

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

protected:
  /// Function to determine the torch data type to be used based on
  /// an input enum
  torch::ScalarType determineTorchDataType(const MooseEnum & data_enum);

  /// We create a vector to store the output of the neural net
  VectorPostprocessorValue & _nn_values;

  /// The data type for the neural net this
  const torch::ScalarType _data_type;
};

#endif
