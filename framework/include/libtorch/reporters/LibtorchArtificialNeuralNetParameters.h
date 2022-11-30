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

#include <torch/torch.h>
#include "LibtorchArtificialNeuralNet.h"
#include "GeneralReporter.h"
#include "nlohmann/json.h"

/**
 * A Reporter which can print the parameter values of a
 * LibtorchArtificialNeuralNetwork from within a Controller object.
 */
class LibtorchArtificialNeuralNetParameters : public GeneralReporter
{
public:
  static InputParameters validParams();

  LibtorchArtificialNeuralNetParameters(const InputParameters & params);

  void initialize() override{};
  void execute() override;
  void finalize() override{};

  void initialSetup() override;

protected:
  /// The name of the control objects which hold the neural networks
  const std::string _control_name;

  /// Pointer to the controller so that we can avoid warehouse lookups in the execute function
  const LibtorchNeuralNetControl * _controller;

  /// Reference to a neural net pointer declared as a reporter.
  /// The parameters of this network are printed into a json file.
  const Moose::LibtorchArtificialNeuralNet *& _network;
};

#endif
