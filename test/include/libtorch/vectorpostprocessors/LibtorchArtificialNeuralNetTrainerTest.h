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

#include "GeneralVectorPostprocessor.h"

/**
 * A UserObject that tests the LibtorchArtificialNeuralNetTrainer.
 */
class LibtorchArtificialNeuralNetTrainerTest : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  LibtorchArtificialNeuralNetTrainerTest(const InputParameters & params);

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

protected:
  // We create a vector to store the output of the neural net
  VectorPostprocessorValue & _nn_values_1;
  VectorPostprocessorValue & _nn_values_2;
};

#endif
