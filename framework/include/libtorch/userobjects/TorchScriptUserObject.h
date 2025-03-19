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

/**
 * A user object the loads a torch module using the
 * torch script format and just-in-time compilation.
 */
class TorchScriptUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TorchScriptUserObject(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};

  ///@{
  /// Get const access to the module pointer.
  const std::unique_ptr<Moose::TorchScriptModule> & modulePtr() const
  {
    return _torchscript_module;
  }
  /// Get const access to the module.
  const Moose::TorchScriptModule & module() const { return *_torchscript_module; }
  /// Get non-const access to the module pointer. Could be used for further training within MOOSE.
  std::unique_ptr<Moose::TorchScriptModule> & modulePtr() { return _torchscript_module; }
  /// Get non-const access to the module. Could be used for further training within MOOSE.
  Moose::TorchScriptModule & module() { return *_torchscript_module; }
  ///@}

  /**
   * Function to evaluate the torch script module at certain input.
   * @param input The input tensor.
   */
  torch::Tensor evaluate(const torch::Tensor & input) const;

protected:
  /// The file name that specifies the torch script model.
  const FileName & _filename;

  /// The libtorch neural network that is currently stored here.
  std::unique_ptr<Moose::TorchScriptModule> _torchscript_module;
};

#endif
