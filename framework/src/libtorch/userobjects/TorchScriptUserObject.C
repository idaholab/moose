//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TorchScriptUserObject.h"
#include "LibtorchUtils.h"

InputParameters
TorchScriptUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::string>("filename", "The file name which contains the saved neural network.");
  params.declareControllable("filename");
  return params;
}

TorchScriptUserObject::TorchScriptUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
  _filename(getParam<std::string>("filename"))
{
}

void TorchScriptUserObject::execute()
{
  _nn = std::make_shared<Moose::LibtorchTorchScriptNeuralNet>(_filename);
}

void TorchScriptUserObject::evaluate(const std::vector<Real> & input, std::vector<Real> & output)
{
  mooseAssert(_nn, "We need a neural network to evalute!");

  torch::Tensor converted_input;
  // This is sad, but the converter function needs this nonconst in libtorch
  std::vector<Real> & nonconst_input = const_cast<std::vector<Real> &>(input);
  LibtorchUtils::vectorToTensor<Real>(nonconst_input, converted_input, /*detach*/true);
  torch::Tensor result = _nn->forward(converted_input);
  LibtorchUtils::tensorToVector<Real>(result, output);
}




