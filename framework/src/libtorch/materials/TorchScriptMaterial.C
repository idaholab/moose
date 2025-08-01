//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "TorchScriptMaterial.h"

registerMooseObject("MooseApp", TorchScriptMaterial);

InputParameters
TorchScriptMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material object which relies on the evaluation of a TorchScript module.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will generate.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "input_names", "The input parameters for the neural network.");
  params.addRequiredParam<UserObjectName>(
      "torch_script_userobject",
      "The name of the user object which contains the torch script module.");

  return params;
}

TorchScriptMaterial::TorchScriptMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _num_props(_prop_names.size()),
    _input_names(getParam<std::vector<PostprocessorName>>("input_names")),
    _num_inputs(_input_names.size()),
    _torch_script_userobject(getUserObject<TorchScriptUserObject>("torch_script_userobject")),
    _input_tensor(torch::zeros(
        {1, _num_inputs},
        torch::TensorOptions().dtype(torch::kFloat64).device(_app.getLibtorchDevice())))
{
  if (!_num_props)
    paramError("prop_names", "Must declare at least one property!");

  if (!_num_inputs)
    paramError("input_names", "Must declare at least one input to the neural net!");

  for (const auto & input_name : _input_names)
    _module_inputs.push_back(&getPostprocessorValueByName(input_name));

  for (const auto & prop_name : _prop_names)
    _properties.push_back(&declareGenericProperty<Real, false>(prop_name));
}

void
TorchScriptMaterial::initQpStatefulProperties()
{
  computeQpValues();
}

void
TorchScriptMaterial::computeQpProperties()
{
  computeQpValues();
}

void
TorchScriptMaterial::computeQpValues()
{
  auto input_accessor = _input_tensor.accessor<Real, 2>();
  for (unsigned int input_i = 0; input_i < _num_inputs; ++input_i)
    input_accessor[0][input_i] = (*_module_inputs[input_i]);

  const auto output = _torch_script_userobject.evaluate(_input_tensor);
  if (_num_props != output.numel())
    mooseError("The tensor needs to be the same length (right now ",
               output.numel(),
               ") as the number of properties (right now ",
               _num_props,
               ")!");

  const auto output_accessor = output.accessor<Real, 2>();
  for (unsigned int prop_i = 0; prop_i < _num_props; ++prop_i)
    (*_properties[prop_i])[_qp] = output_accessor[0][prop_i];
}

#endif
