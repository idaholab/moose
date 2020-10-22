//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "OptimizationParameterTransfer.h"
#include "MultiApp.h"
#include "ControlsReceiver.h"
#include "OptimizationParameterVectorPostprocessor.h"

registerMooseObject("isopodApp", OptimizationParameterTransfer);

InputParameters
OptimizationParameterTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription("Copies optimization data to a ControlsReceiver object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "parameter_vpp",
      "The name of the OptimizationParameterVectorPostprocessor to get data "
      "from");
  params.addRequiredParam<std::string>("to_control",
                                       "The name of the 'ControlsReceiver' on the sub application "
                                       "to which the optimization parameters will be transferred.");

  params.set<MultiMooseEnum>("direction") = "to_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");

  return params;
}

OptimizationParameterTransfer::OptimizationParameterTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("parameter_vpp")),
    _receiver_name(getParam<std::string>("to_control"))
{
}

void
OptimizationParameterTransfer::execute()
{
  auto & vpp =
      _multi_app->problemBase().getUserObject<OptimizationParameterVectorPostprocessor>(_vpp_name);
  std::vector<std::string> parameter_names(vpp.getParameterNames());
  std::vector<Real> parameter_values(vpp.getParameterValues());

  ControlsReceiver * ptr = getReceiver(processor_id());
  ptr->transfer(parameter_names, parameter_values);
}

ControlsReceiver *
OptimizationParameterTransfer::getReceiver(unsigned int app_index)
{
  // Test that the sub-application has the given Control object
  FEProblemBase & to_problem = _multi_app->appProblemBase(app_index);
  ExecuteMooseObjectWarehouse<Control> & control_wh = to_problem.getControlWarehouse();
  if (!control_wh.hasActiveObject(_receiver_name))
    mooseError("The sub-application (",
               _multi_app->name(),
               ") does not contain a Control object with the name '",
               _receiver_name,
               "'.");

  ControlsReceiver * ptr =
      dynamic_cast<ControlsReceiver *>(control_wh.getActiveObject(_receiver_name).get());

  if (!ptr)
    mooseError(
        "The sub-application (",
        _multi_app->name(),
        ") Control object for the 'to_control' parameter must be of type 'ControlsReceiver'.");

  return ptr;
}
