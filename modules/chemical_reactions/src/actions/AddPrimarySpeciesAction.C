/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddPrimarySpeciesAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

template <>
InputParameters
validParams<AddPrimarySpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "primary_species", "The list of primary variables to add");
  return params;
}

AddPrimarySpeciesAction::AddPrimarySpeciesAction(const InputParameters & params)
  : Action(params), _vars(getParam<std::vector<NonlinearVariableName>>("primary_species"))
{
}

void
AddPrimarySpeciesAction::act()
{
  for (unsigned int i = 0; i < _vars.size(); ++i)
  {
    FEType fe_type(Utility::string_to_enum<Order>("first"),
                   Utility::string_to_enum<FEFamily>("lagrange"));

    Real scale_factor = 1.0;
    _problem->addVariable(_vars[i], fe_type, scale_factor);
  }
}
