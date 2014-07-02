#include "ReconVarICAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"

template<>
InputParameters validParams<ReconVarICAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<unsigned int>("crys_num", "Specifies the number of order paraameters to create");
  params.addRequiredParam<unsigned int>("grain_num", "Specifies the number of grains in the reconstructed dataset");
  params.addRequiredParam<std::string>("var_name_base","specifies the base name of the variables");
  return params;
}

ReconVarICAction::ReconVarICAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _crys_num(getParam<unsigned int>("crys_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{}

void
ReconVarICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the ReconVarICAction Object\n";
#endif
  
  // Set initial condition for each order parameter
  for (unsigned int crys = 0; crys < _crys_num; ++crys)
  {
    // Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << crys;
    var_name.append(out.str());
    
    {
      // Define parameters for ReconVarIC
      InputParameters poly_params = _factory.getValidParams("ReconVarIC");
      poly_params.applyParameters(_pars);
      poly_params.set<VariableName>("variable") = var_name;
      poly_params.set<unsigned int>("crys_index") = crys;
      //poly_params.set<std::vector<VariableName> >("eta") = getParam<std::vector<VariableName> >("eta");

      // Add initial condition
      _problem->addInitialCondition("ReconVarIC", "Initialize_op", poly_params);
    }
  }
}
