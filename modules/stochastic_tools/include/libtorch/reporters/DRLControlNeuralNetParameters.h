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

#include "LibtorchArtificialNeuralNet.h"
#include "SurrogateModelInterface.h"
#include "GeneralReporter.h"

/**
 * A Reporter which can print the parameter values of a
 * LibtorchArtificialNeuralNetwork from within a DRL Trainer object.
 */
class DRLControlNeuralNetParameters : public GeneralReporter, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  DRLControlNeuralNetParameters(const InputParameters & params);

  void initialize() override{};
  void execute() override{};
  void finalize() override{};
};

#endif
