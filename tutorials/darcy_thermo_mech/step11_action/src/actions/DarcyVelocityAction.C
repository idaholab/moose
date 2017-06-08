/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// libMesh includes
#include "libmesh/string_to_enum.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "MooseEnum.h"
#include "FEProblemBase.h"

// Step 11 includes
#include "DarcyVelocityAction.h"

template <>
InputParameters
validParams<DarcyVelocityAction>()
{
  // Define MooseEnum for common order settings that can be set to values outside of the
  // specified list.
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH", "CONSTANT", true);

  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed).");

  params.addParam<std::string>("variable_name", "velocity", "The prefix to utilize for the "
                                                            "velocity name, the resulting "
                                                            "variables will append '_x', '_y', and "
                                                            "'_z' for the variable components.");
  params.addRequiredCoupledVar("darcy_pressure", "The pressure field.");

  params.addParam<MultiMooseEnum>(
      "execute_on",
      Output::getExecuteOptions("initial timestep_end"),
      "Set to (initial|linear|nonlinear|timestep_end|timestep_begin|final|failed|custom) to "
      "execute only at that moment (default: 'initial timestep_end')");

  return params;
}

DarcyVelocityAction::DarcyVelocityAction(InputParameters parameters) : Action(parameters) {}

void
DarcyVelocityAction::act()
{
  const std::string & prefix = getParam<std::string>("variable_name");

  // Adds AuxVariables
  if (_current_task == "add_aux_variable")
  {
    FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")), MONOMIAL);
    _problem->addAuxVariable(prefix + "_x", fe_type);
    _problem->addAuxVariable(prefix + "_y", fe_type);
    _problem->addAuxVariable(prefix + "_z", fe_type);
  }

  // Adds AuxKernels
  else if (_current_task == "add_aux_kernel")
  {
    // Get the validParam for the AuxKernel
    const std::string type = "DarcyVelocity";
    InputParameters params = _factory.getValidParams("DarcyVelocity");

    // Apply the supplied pressure variable name
    params.set<std::vector<VariableName>>("darcy_pressure") =
        getParam<std::vector<VariableName>>("darcy_pressure");
    params.set<MultiMooseEnum>("execute_on") = getParam<MultiMooseEnum>("execute_on");

    // Create the AuxKernels
    params.set<MooseEnum>("component") = "x";
    params.set<AuxVariableName>("variable") = {prefix + "_x"};
    _problem->addAuxKernel(type, prefix + "_x", params);

    params.set<MooseEnum>("component") = "y";
    params.set<AuxVariableName>("variable") = {prefix + "_y"};
    _problem->addAuxKernel(type, prefix + "_y", params);

    params.set<MooseEnum>("component") = "z";
    params.set<AuxVariableName>("variable") = {prefix + "_y"};
    _problem->addAuxKernel(type, prefix + "_z", params);
  }
}
