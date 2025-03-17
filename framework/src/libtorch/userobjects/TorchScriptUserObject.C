//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TorchScriptUserObject.h"

registerMooseObject("MooseApp", TorchScriptUserObject);

InputParameters
TorchScriptUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("User-facing object which loads a torch script module.");
  params.addRequiredParam<std::string>("filename", "The file name which contains the torch script module.");
  params.declareControllable("filename");
  params.addParam<bool>("load_during_construction", false, "If we want to load this neural network while we are constructing this object.");
  return params;
}

TorchScriptUserObject::TorchScriptUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
  _filename(getParam<std::string>("filename")),
  _torchscript_module(getParam<bool>("load_during_construction") ? std::make_shared<Moose::TorchScriptModule>(_filename) : nullptr)
{
}

void TorchScriptUserObject::execute()
{
  // We load when the user executes this user object
  _torchscript_module = std::make_shared<Moose::TorchScriptModule>(_filename);
}

torch::Tensor TorchScriptUserObject::evaluate(torch::Tensor & input) const
{
  if (!_torchscript_module)
    mooseError("The torch script module has not been loaded yet, so it can't be evaluated! Make sure that the torch script module is loaded (execute_on flags) before calling this function!");
  return _torchscript_module->forward(input);
}




