//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppFieldTransferInterface.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MultiAppFieldTransferInterface>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<bool>("preserve_transfer",
                        false,
                        "Whether or not to conserve the transfered field, "
                        " if true, the transfered variables will be adjusted "
                        "according to the pps value");

  params.addParam<PostprocessorName>(
      "from_postprocessor_to_be_preserved",
      "from_postprocessor",
      "The name of the Postprocessor in the from-app  to evaluate an adjusting factor.");

  params.addParam<PostprocessorName>(
      "to_postprocessor_to_be_preserved",
      "to_postprocessor",
      "The name of the Postprocessor in the to-app to evaluate an adjusting factor.");
  return params;
}

MultiAppFieldTransferInterface::MultiAppFieldTransferInterface(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_var_name(getParam<VariableName>("source_variable")),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _preserve_transfer(parameters.get<bool>("preserve_transfer")),
    _from_postprocessor_to_be_preserved(
        parameters.get<PostprocessorName>("from_postprocessor_to_be_preserved")),
    _to_postprocessor_to_be_preserved(
        parameters.get<PostprocessorName>("to_postprocessor_to_be_preserved"))
{
}

void
MultiAppFieldTransferInterface::postExecute()
{
  if (_preserve_transfer)
  {
    _console << "Beginning MultiAppCopyTransfer postExecute " << name() << std::endl;

    if (_direction == TO_MULTIAPP)
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
          adjustTransferedSolution(from_problem, _multi_app->appProblemBase(i));
    }

    else if (_direction == FROM_MULTIAPP)
    {
      FEProblemBase & to_problem = _multi_app->problemBase();
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
          adjustTransferedSolution(_multi_app->appProblemBase(i), to_problem);
    }

    _console << "Finished MultiAppCopyTransfer postExecute " << name() << std::endl;
  }
}

void
MultiAppFieldTransferInterface::adjustTransferedSolution(FEProblemBase & from_problem,
                                                         FEProblemBase & to_problem)
{
  PostprocessorValue & from_adjuster =
      from_problem.getPostprocessorValue(_from_postprocessor_to_be_preserved);
  // Compute to-postproessor to have the adjuster
  to_problem.computeUserObjectByName(EXEC_TRANSFER, _to_postprocessor_to_be_preserved);

  // Now we should have the right adjuster based on the transfered solution
  PostprocessorValue & to_adjuster =
      to_problem.getPostprocessorValue(_to_postprocessor_to_be_preserved);

  auto & to_var = to_problem.getVariable(
      0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

  // Scale the solution.
  to_var.sys().solution().scale(from_adjuster / to_adjuster);
  // Update the local solution
  to_var.sys().update();
}
