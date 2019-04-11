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
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "The variable to transfer from.");
  params.addParam<bool>("preserve_transfer",
                        false,
                        "Whether or not to conserve the transfered field, "
                        " if true, the transfered variables will be adjusted "
                        "according to the pps value");

  std::vector<PostprocessorName> from_postprocessor = {"from_postprocessor"};
  params.addParam<std::vector<PostprocessorName>>(
      "from_postprocessor_to_be_preserved",
      from_postprocessor,
      "The name of the Postprocessor in the from-app to evaluate an adjusting factor.");

  std::vector<PostprocessorName> to_postprocessor = {"to_postprocessor"};
  params.addParam<std::vector<PostprocessorName>>(
      "to_postprocessor_to_be_preserved",
      to_postprocessor,
      "The name of the Postprocessor in the to-app to evaluate an adjusting factor.");
  return params;
}

MultiAppFieldTransferInterface::MultiAppFieldTransferInterface(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_var_name(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_name(getParam<std::vector<AuxVariableName>>("variable")),
    _preserve_transfer(parameters.get<bool>("preserve_transfer")),
    _from_postprocessor_to_be_preserved(
        parameters.get<std::vector<PostprocessorName>>("from_postprocessor_to_be_preserved")),
    _to_postprocessor_to_be_preserved(
        parameters.get<std::vector<PostprocessorName>>("to_postprocessor_to_be_preserved"))
{
  if (_preserve_transfer)
  {
    if (_direction == TO_MULTIAPP)
    {
      mooseAssert(_from_postprocessor_to_be_preserved.size() == _multi_app->numGlobalApps(),
                  "Number of from Postprocessors should equal to the number of subapps");
      mooseAssert(_to_postprocessor_to_be_preserved.size() == 1,
                  "Number of to Postprocessors should equal to 1");
    }
    else if (_direction == FROM_MULTIAPP)
    {
      mooseAssert(_from_postprocessor_to_be_preserved.size() == 1,
                  "Number of from Postprocessors should equal to 1");
      mooseAssert(_to_postprocessor_to_be_preserved.size() == _multi_app->numGlobalApps(),
                  "Number of to Postprocessors should equal to the number of subapps ");
    }
  }
}

void
MultiAppFieldTransferInterface::postExecute()
{
  if (_preserve_transfer)
  {
    _console << "Beginning Conservative transfers " << name() << std::endl;

    if (_direction == TO_MULTIAPP)
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
          adjustTransferedSolution(from_problem,
                                   _from_postprocessor_to_be_preserved[i],
                                   _multi_app->appProblemBase(i),
                                   _to_postprocessor_to_be_preserved[0]);
    }

    else if (_direction == FROM_MULTIAPP)
    {
      FEProblemBase & to_problem = _multi_app->problemBase();
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
          adjustTransferedSolution(_multi_app->appProblemBase(i),
                                   _from_postprocessor_to_be_preserved[0],
                                   to_problem,
                                   _to_postprocessor_to_be_preserved[i]);
    }

    _console << "Finished Conservative transfers " << name() << std::endl;
  }
}

void
MultiAppFieldTransferInterface::adjustTransferedSolution(FEProblemBase & from_problem,
                                                         PostprocessorName & from_postprocessor,
                                                         FEProblemBase & to_problem,
                                                         PostprocessorName & to_postprocessor)
{
  PostprocessorValue & from_adjuster = from_problem.getPostprocessorValue(from_postprocessor);
  // Compute to-postproessor to have the adjuster
  to_problem.computeUserObjectByName(EXEC_TRANSFER, to_postprocessor);

  std::cout << "from_postprocessor " << from_postprocessor << std::endl;
  std::cout << "to_postprocessor " << to_postprocessor << std::endl;

  // Now we should have the right adjuster based on the transfered solution
  PostprocessorValue & to_adjuster = to_problem.getPostprocessorValue(to_postprocessor);

  auto & to_var = to_problem.getVariable(
      0, _to_var_name[0], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

  std::cout << "from_adjuster " << from_adjuster << std::endl;
  std::cout << "to_adjuster " << to_adjuster << std::endl;

  // Scale the solution.
  // to_var.sys().solution().scale(from_adjuster / to_adjuster);
  to_var.sys().solution().scale(from_adjuster / to_adjuster);
  // Update the local solution
  to_var.sys().update();
}
