//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html


#include "NeuralNetworkIC.h"
#include "MooseMesh.h"

registerMooseObject("StochasticToolsApp", NeuralNetworkIC);

InputParameters
NeuralNetworkIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("Initializes variable using a neural network UserObject");
  params.addRequiredParam<UserObjectName>(
      "NeuralNetwork_user_object",
      "Name of the neural network user object that evaluates the nodal value");
  params.addRequiredCoupledVar("InputVariables", "Names of the non-linear variables for inputting to the neural net");
  params.addParam<std::size_t>("op_id",0,"Index of the output neuron that is used by the IC");
  return params;
}

NeuralNetworkIC::NeuralNetworkIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _nn_obj(getUserObject<NeuralNetworkUserObject>("NeuralNetwork_user_object")),
    _op_id(getParam<std::size_t>("op_id"))
{
  _depend_vars.insert(name());
  _n_inputs = coupledComponents("InputVariables");

  _input_vect.resize(_n_inputs);
  for (unsigned int i = 0; i < _n_inputs; ++i)
    _input_vect[i] = &coupledValue("InputVariables", i);
}

Real
NeuralNetworkIC::value(const Point & p)
{
  DenseVector<Real> _input_layer(_n_inputs);
  for (unsigned int i = 0; i < _n_inputs; ++i)
  {
    auto * temp = _input_vect[i];
    _input_layer(i) = temp[0][0];
  }
  return _nn_obj.evaluate(_input_layer, _op_id);
}

const std::set<std::string> &
NeuralNetworkIC::getRequestedItems()
{
  return _depend_vars;
}
