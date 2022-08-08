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
#include "SurrogateModelInterface.h"

/**
 * A VectorPostprocessor which can print the parameter values of a
 * LibtorchArtificialNeuralNetwork from within a DRL Trainer object.
 */
class DRLControlNeuralNetParameters : public LibtorchArtificialNeuralNetParameters, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  DRLControlNeuralNetParameters(const InputParameters & params);

  virtual void initialize(){};
  virtual void execute() override;
  virtual void finalize(){};

protected:
  /// The name of the DRL trainer objects which hold the neural networks
  UserObjectName _trainer_name;
};
