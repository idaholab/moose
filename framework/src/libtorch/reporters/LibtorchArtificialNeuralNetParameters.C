//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchNeuralNetControl.h"
#include "LibtorchArtificialNeuralNetParameters.h"

registerMooseObject("MooseApp", LibtorchArtificialNeuralNetParameters);

InputParameters
LibtorchArtificialNeuralNetParameters::validParams()
{
  InputParameters params = GeneralReporter::validParams();

  params.addClassDescription("Outputs the parameters of a LibtorchArtificialNeuralNetwork within a "
                             "LibtorchNeuralNetControl.");

  params.addRequiredParam<std::string>("control_name",
                                       "The control object holding the neural network.");

  return params;
}

LibtorchArtificialNeuralNetParameters::LibtorchArtificialNeuralNetParameters(
    const InputParameters & params)
  : GeneralReporter(params),
    _control_name(getParam<std::string>("control_name")),
    _network(
        declareValueByName<const Moose::LibtorchArtificialNeuralNet *>(name(), REPORTER_MODE_ROOT))
{
}

void
LibtorchArtificialNeuralNetParameters::initialSetup()
{
  auto & control_warehouse = _fe_problem.getControlWarehouse();
  const auto & control_ref = control_warehouse.getActiveObject(_control_name);
  _controller = dynamic_cast<const LibtorchNeuralNetControl *>(control_ref.get());
}

void
LibtorchArtificialNeuralNetParameters::execute()
{
  // We update the network link in the reporter so it always prints the latest network, this matters
  // when the network in the controller is trained on the fly
  _network =
      dynamic_cast<const Moose::LibtorchArtificialNeuralNet *>(&(_controller->controlNeuralNet()));
}

#endif
