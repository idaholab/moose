#include "AddPrimarySpeciesAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"
#include "string_to_enum.h"
#include "fe.h"


template<>
InputParameters validParams<AddPrimarySpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName> >("primary_species", "The list of primary variables to add");

  return params;
}


AddPrimarySpeciesAction::AddPrimarySpeciesAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddPrimarySpeciesAction::act()
{
  std::vector<NonlinearVariableName> vars = getParam<std::vector<NonlinearVariableName> >("primary_species");

  for (unsigned int i=0; i < vars.size(); i++)
  {
    FEType fe_type(Utility::string_to_enum<Order>("first"),
                   Utility::string_to_enum<FEFamily>("lagrange"));
    Real scale_factor = 1.0;
    _problem->addVariable(vars[i], fe_type, scale_factor);
  }

}
