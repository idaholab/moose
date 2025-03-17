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
#include "TorchScriptModule.h"


class TorchScriptUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TorchScriptUserObject(const InputParameters & parameters);

  virtual void initialize() override {};
  virtual void execute() override;
  virtual void finalize() override {};

  const std::shared_ptr<Moose::TorchScriptModule> & module() const {return _torchscript_module;}

  /**
   * Function to evaluate the torch script module at certain input.
   * @param input The input tensor. Unfortunately, this cannot be const since it creates a graph in the background.
   */
  torch::Tensor evaluate(torch::Tensor & input) const;

protected:

  /// The file name that specifies the torch script model
  const std::string & _filename;

  /// The libtorch neural network that is currently stored here
  std::shared_ptr<Moose::TorchScriptModule> _torchscript_module;
};

#endif
