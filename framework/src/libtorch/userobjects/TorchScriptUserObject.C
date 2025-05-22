//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "TorchScriptUserObject.h"

registerMooseObject("MooseApp", TorchScriptUserObject);

InputParameters
TorchScriptUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("User-facing object which loads a torch script module.");
  params.addRequiredParam<FileName>("filename",
                                    "The file name which contains the torch script module.");
  params.declareControllable("filename");
  params.addParam<bool>(
      "load_during_construction",
      false,
      "If we want to load this neural network while we are constructing this object.");

  // By default we don't execute this user object, depending on the desired reload frequency,
  // the user can override this in the input file.
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_NONE};

  return params;
}

TorchScriptUserObject::TorchScriptUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _filename(getParam<FileName>("filename")),
    _torchscript_module(std::make_unique<Moose::TorchScriptModule>(_filename))
{
}

void
TorchScriptUserObject::execute()
{
  // We load when the user executes this user object
  _torchscript_module = std::make_unique<Moose::TorchScriptModule>(_filename);
}

torch::Tensor
TorchScriptUserObject::evaluate(const torch::Tensor & input) const
{
  return _torchscript_module->forward(input);
}

#endif
