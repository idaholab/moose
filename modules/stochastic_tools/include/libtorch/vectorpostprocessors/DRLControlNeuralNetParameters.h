//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LibtorchArtificialNeuralNetParameters.h"
#include "LibtorchArtificialNeuralNet.h"

#include "SurrogateModelInterface.h"
#include "GeneralVectorPostprocessor.h"

/**
 * A VectorPostprocessor which can print the parameter values of a
 * LibtorchArtificialNeuralNetwork from within a DRL Trainer object.
 */
class DRLControlNeuralNetParameters : public GeneralVectorPostprocessor,
                                      public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  DRLControlNeuralNetParameters(const InputParameters & params);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};

protected:
  /// A vector which stores the parameters of the neural net
  VectorPostprocessorValue & _nn_parameter_values;

  /// Pointer to the neural network
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _ann;
};
