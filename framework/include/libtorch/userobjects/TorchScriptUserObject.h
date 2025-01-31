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

// MOOSE includes
#include "GeneralUserObject.h"
#include "LibtorchTorchScriptNeuralNet.h"


class TorchScriptUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TorchScriptUserObject(const InputParameters & parameters);

  virtual void initialize() override {};
  virtual void execute() override;
  virtual void finalize() override {};

  const std::shared_ptr<Moose::LibtorchTorchScriptNeuralNet> & network() {return _nn;}

  /**
   * Function to evaluate the neural network at certain input
   * @param input The input vector (why not const, because torch needs nonconst sadly)
   * @param output Storage for the outputs
   */
  torch::Tensor evaluate(torch::Tensor & input) const;

protected:

  /// The file name that specifies the torch script model
  const std::string & _filename;

  /// The libtorch neural network that is currently stored here
  std::shared_ptr<Moose::LibtorchTorchScriptNeuralNet> _nn;
};

#endif
