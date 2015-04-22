#include "ReconVarICAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Conversion.h"

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
  params.addRequiredParam<unsigned int>("op_num", "Specifies the number of order paraameters to create");
  params.addRequiredParam<std::string>("var_name_base","specifies the base name of the variables");
  params.addRequiredParam<bool>("consider_phase","If true, IC will only act on one phase");
  params.addParam<unsigned int>("phase", 0,"EBSD phase number to be assigned to this grain");

  return params;
}

ReconVarICAction::ReconVarICAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{}

void
ReconVarICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the ReconVarICAction Object\n";
#endif

  // Set initial condition for each order parameter
  for (unsigned int op = 0; op < _op_num; ++op)
  {
    // Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << op;
    var_name.append(out.str());

    {
      // Define parameters for ReconVarIC
      InputParameters poly_params = _factory.getValidParams("ReconVarIC");
      poly_params.set<VariableName>("variable") = var_name;
      poly_params.set<unsigned int>("op_index") = op;
      poly_params.set<unsigned int>("op_num") = _op_num;
      poly_params.set<UserObjectName>("ebsd_reader") = getParam<UserObjectName>("ebsd_reader");
      poly_params.set<bool>("consider_phase") = getParam<bool>("consider_phase");
      poly_params.set<unsigned int>("phase") = getParam<unsigned int>("phase");
      poly_params.set<bool>("all_to_one") = false;

      // Add initial condition
      _problem->addInitialCondition("ReconVarIC", "ICs/Initialize_op_" + Moose::stringify(op), poly_params);
    }
  }
}
