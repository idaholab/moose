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
#include "SamplerReceiver2.h"

registerMooseObject("StochasticToolsApp", OptimizationTransfer);

InputParameters
OptimizationTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription("Copies optimization data to a SamplerReceiver object.");
  params.set<MultiMooseEnum>("direction") = "to_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");
  params.addParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control "
      "with the Sampler data.");
  params.addParam<std::vector<Real>>(
      "guess_values",
      "A list of guess values for the starting point of the optimization "
      "for each parameter.");
  params.addRequiredParam<std::string>("to_control",
                                       "The name of the 'SamplerReceiver' on the sub application "
                                       "to which the parameters data will be transferred.");
  return params;
}

OptimizationTransfer::OptimizationTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _parameter_names(getParam<std::vector<std::string>>("parameters")),
    _guess_values(getParam<std::vector<Real>>("guess_values")),
    _receiver_name(getParam<std::string>("to_control"))
{
  if (_parameter_names.size() != _guess_values.size())
    mooseError("In OptimizationTransfer, there should be one guess_value for each parameter.\n"
               "    Number of parameters =",
               _parameter_names.size(),
               "\n"
               "    Number of guess_values =",
               _guess_values.size());
}

void
OptimizationTransfer::execute()
{
  std::cout << "**********OptimizationTransfer::execute()"
            << "    INitilize with guess " << std::endl;
  static Real ddummyValue1 = _guess_values[0];
  static Real ddummyValue2 = _guess_values[1];
  std::vector<Real> row = {ddummyValue1, ddummyValue2}; // fixme lynn hard coded values for now
  for (size_t i = 0; i < _parameter_names.size(); ++i)
    std::cout << "         -------NAMES = " << _parameter_names[i] << "   VALUE= " << row[i]
              << std::endl;

  SamplerReceiver2 * ptr = getReceiver(processor_id());
  ptr->transfer(_parameter_names, row);
}

void
OptimizationTransfer::executeToMultiapp()
{
  static Real dummyValue1 = _guess_values[0];
  static Real dummyValue2 = _guess_values[1];
  dummyValue1 += 20;
  dummyValue2 -= 50;
  std::vector<Real> row = {dummyValue1, dummyValue2}; // fixme lynn hard coded values for now
  std::cout << "---------------OptimizationTransfer::executeToMultiapp()  Number of parameters= "
            << _parameter_names.size() << std::endl;
  for (size_t i = 0; i < _parameter_names.size(); ++i)
    std::cout << "         -------NAMES = " << _parameter_names[i] << "   VALUE= " << row[i]
              << std::endl;

  // fixme lynn ,part to actually use
  SamplerReceiver2 * ptr = getReceiver(processor_id());
  ptr->transfer(_parameter_names, row);
}

SamplerReceiver2 *
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

  SamplerReceiver2 * ptr =
      dynamic_cast<SamplerReceiver2 *>(control_wh.getActiveObject(_receiver_name).get());

  if (!ptr)
    mooseError(
        "The sub-application (",
        _multi_app->name(),
        ") Control object for the 'to_control' parameter must be of type 'SamplerReceiver'.");

  return ptr;
}
