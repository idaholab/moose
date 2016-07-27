/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddNavierStokesVariablesAction.h"
#include "NavierStokesApp.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

template<>
InputParameters validParams<AddNavierStokesVariablesAction>()
{
  InputParameters params = validParams<Action>();

  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies(), "LAGRANGE");
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders(), "FIRST");
  params.addParam<MooseEnum>("family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of the FE shape function to use for this variable (additional orders not listed are allowed)");
  params.addRequiredParam<std::vector<Real> >("scaling", "Specifies a scaling factor to apply to this variable");

  return params;
}

AddNavierStokesVariablesAction::AddNavierStokesVariablesAction(InputParameters parameters) :
    Action(parameters),
    _scaling(getParam<std::vector<Real> >("scaling"))
{
}

AddNavierStokesVariablesAction::~AddNavierStokesVariablesAction()
{
}

void
AddNavierStokesVariablesAction::act()
{
  unsigned int dim = _mesh->dimension();

  // Build up the vector of variable names for the user, depending on
  // the mesh dimension.
  std::vector<NonlinearVariableName> names;
  names.push_back("rho");
  names.push_back("rhou");
  if (dim >= 2)
    names.push_back("rhov");
  if (dim >= 3)
    names.push_back("rhow");
  names.push_back("rhoE");

  // Make sure the number of scaling parameters matches the number of variables
  if (_scaling.size() != names.size())
    mooseError("Must provide a scaling parameter for each variable.");

  // All variables have the same type
  FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));


  // Add the variables to the FEProblem
  for (unsigned int i = 0; i < names.size(); ++i)
    _problem->addVariable(names[i], fe_type, _scaling[i]);

  // Add Aux variables.  These are all required in order for the code
  // to run, so they should not be independently selectable by the
  // user.

  std::vector<AuxVariableName> aux_names;
  aux_names.push_back("vel_x");
  if (dim >= 2)
    aux_names.push_back("vel_y");
  if (dim >= 3)
    aux_names.push_back("vel_z");

  aux_names.push_back("pressure");
  aux_names.push_back("temperature");
  aux_names.push_back("enthalpy");
  aux_names.push_back("Mach");

  // Needed for FluidProperties calculations
  aux_names.push_back("internal_energy");
  aux_names.push_back("specific_volume");

 for (unsigned int i = 0; i < aux_names.size(); ++i)
   _problem->addAuxVariable(aux_names[i], fe_type);
}
