//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "OptimizationTransfer.h"
#include "MultiApp.h"
#include "SamplerReceiver.h"
#include "OptimizationVectorPostprocessor.h"

registerMooseObject("isopodApp", OptimizationTransfer);

InputParameters
OptimizationTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription("Copies optimization data to a SamplerReceiver object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "optimization_vpp",
      "The name of the OptimizationVectorPostprocessor VectorPostprocessor to get data from");
  params.addRequiredParam<std::string>("to_control",
                                       "The name of the 'SamplerReceiver' on the sub application "
                                       "to which the optimization parameters will be transferred.");

  params.set<MultiMooseEnum>("direction") = "to_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");

  return params;
}

OptimizationTransfer::OptimizationTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("optimization_vpp")),
    _receiver_name(getParam<std::string>("to_control"))
{
}

void
OptimizationTransfer::execute()
{
  auto & vpp = _multi_app->problemBase().getUserObject<OptimizationVectorPostprocessor>(_vpp_name);
  std::vector<std::string> parameter_names(vpp.getParameterNames());
  std::vector<Real> parameter_values(vpp.getParameterValues());

  SamplerReceiver * ptr = getReceiver(processor_id());
  ptr->transfer(parameter_names, parameter_values);
}

SamplerReceiver *
OptimizationTransfer::getReceiver(unsigned int app_index)
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

  SamplerReceiver * ptr =
      dynamic_cast<SamplerReceiver *>(control_wh.getActiveObject(_receiver_name).get());

  if (!ptr)
    mooseError(
        "The sub-application (",
        _multi_app->name(),
        ") Control object for the 'to_control' parameter must be of type 'SamplerReceiver'.");

  return ptr;
}
