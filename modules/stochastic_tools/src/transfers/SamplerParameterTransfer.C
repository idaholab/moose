//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SamplerParameterTransfer.h"
#include "SamplerTransientMultiApp.h"
#include "SamplerFullSolveMultiApp.h"
#include "ParameterReceiver.h"
#include "Sampler.h"

registerMooseObjectRenamed("StochasticToolsApp",
                           SamplerTransfer,
                           "01/01/2020 00:00",
                           SamplerParameterTransfer);
registerMooseObject("StochasticToolsApp", SamplerParameterTransfer);

InputParameters
SamplerParameterTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Copies Sampler data to a ParameterReceiver object.");
  params.set<MultiMooseEnum>("direction") = "to_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");
  params.addParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control "
      "with the Sampler data. The order of the parameters listed "
      "here should match the order of the items in the Sampler.");
  params.addRequiredParam<std::string>("to_control",
                                       "The name of the 'ParameterReceiver' on the sub application "
                                       "to which the Sampler data will be transferred.");
  return params;
}

SamplerParameterTransfer::SamplerParameterTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    ParameterReceiverInterface(),
    _parameter_names(getParam<std::vector<std::string>>("parameters")),
    _receiver_name(getParam<std::string>("to_control"))
{
}

void
SamplerParameterTransfer::execute()
{
  mooseAssert((_sampler_ptr->getNumberOfLocalRows() == 0) ||
                  (_sampler_ptr->getNumberOfLocalRows() == _multi_app->numLocalApps()),
              "The number of MultiApps and the number of sample rows must be the same.");

  // Loop over all sub-apps
  for (dof_id_type row_index = _sampler_ptr->getLocalRowBegin();
       row_index < _sampler_ptr->getLocalRowEnd();
       row_index++)
  {
    mooseAssert(_multi_app->hasLocalApp(row_index),
                "The current sample row index is not a valid global MultiApp index.");

    // Get the sub-app ParameterReceiver object and perform error checking
    ParameterReceiver * ptr = getReceiver(row_index);

    // Populate the row of data to transfer
    std::vector<Real> row = _sampler_ptr->getNextLocalRow();

    // Perform the transfer
    transferParameters(*ptr, _parameter_names, row);
  }
}

void
SamplerParameterTransfer::initializeToMultiapp()
{
  _global_index = _sampler_ptr->getLocalRowBegin();
}

void
SamplerParameterTransfer::executeToMultiapp()
{
  ParameterReceiver * ptr = getReceiver(processor_id());

  std::vector<Real> row = _sampler_ptr->getNextLocalRow();

  // Perform the transfer
  transferParameters(*ptr, _parameter_names, row);

  _global_index++;
}

void
SamplerParameterTransfer::finalizeToMultiapp()
{
}

ParameterReceiver *
SamplerParameterTransfer::getReceiver(unsigned int app_index)
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

  ParameterReceiver * ptr =
      dynamic_cast<ParameterReceiver *>(control_wh.getActiveObject(_receiver_name).get());

  if (!ptr)
    mooseError(
        "The sub-application (",
        _multi_app->name(),
        ") Control object for the 'to_control' parameter must be of type 'ParameterReceiver'.");

  return ptr;
}
