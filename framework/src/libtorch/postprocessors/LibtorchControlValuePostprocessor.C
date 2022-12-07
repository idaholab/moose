//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchControlValuePostprocessor.h"

registerMooseObject("MooseApp", LibtorchControlValuePostprocessor);

InputParameters
LibtorchControlValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription("Reports the value stored in given controlled parameters.");

  params.addRequiredParam<std::string>("control_name",
                                       "The name of the LibtorchNeuralNetControl object.");
  params.addParam<unsigned int>("signal_index",
                                0,
                                "The index of the signal from the LibtorchNeuralNetControl object. "
                                "This assumes indexing between [0,num_signals).");
  return params;
}

LibtorchControlValuePostprocessor::LibtorchControlValuePostprocessor(const InputParameters & params)
  : GeneralPostprocessor(params), _signal_index(getParam<unsigned int>("signal_index"))

{
}

void
LibtorchControlValuePostprocessor::initialSetup()
{
  GeneralPostprocessor::initialSetup();

  _libtorch_nn_control = dynamic_cast<LibtorchNeuralNetControl *>(
      _fe_problem.getControlWarehouse()
          .getActiveObject(getParam<std::string>("control_name"))
          .get());
  if (!_libtorch_nn_control)
    paramError("control_name",
               "The supplied control object is not derived from LibtorchNeuralNetControl!");

  if (_libtorch_nn_control->numberOfControlSignals() <= _signal_index)
    paramError("signal_index",
               "The given control object only has ",
               _libtorch_nn_control->numberOfControlSignals(),
               " control signals!");
}

Real
LibtorchControlValuePostprocessor::getValue()
{
  // Return the value of the control signal
  return _libtorch_nn_control->getSignal(_signal_index);
}

#endif
