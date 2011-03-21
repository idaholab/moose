#include "AddVariableAction.h"
#include "Parser.h"
#include "MProblem.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"
#include "string_to_enum.h"

// class static initializiation
const Real AddVariableAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<AddVariableAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("family", "LAGRANGE", "Specifies the family of FE shape functions to use for this variable");
  params.addParam<std::string>("order", "FIRST",  "Specifies the order of the FE shape function to use for this variable");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParam<int>("initial_from_file_timestep", 2, "Gives the timestep for which to read a solution from a file for a given variable");
  params.addParam<std::string>("initial_from_file_var", "Gives the name of a variable for which to read an initial condition from a mesh file");
  params.addParam<std::vector<unsigned int> >("block", "The block id where this variable lives");

  return params;
}


AddVariableAction::AddVariableAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _variable_to_read(""),
    _timestep_to_read(2)
  
{
   std::cerr << "Constructing AddVariableAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddVariableAction::act()
{
  std::cerr << "Acting on AddVariableAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";

  std::string var_name = getShortName();
  bool is_variables_block;

  MProblem *prob = _parser_handle._problem;

  is_variables_block = Parser::pathContains(_name, "Variables");
  if (is_variables_block)
  {

    std::set<subdomain_id_type> blocks;
    std::vector<unsigned int> block_param = getParam<std::vector<unsigned int> >("block");
    for (std::vector<unsigned int>::iterator it = block_param.begin(); it != block_param.end(); ++it)
      blocks.insert(*it);

    if (blocks.empty())
	{
      prob->addVariable(var_name,
                        FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                               Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))));
	}
	else
      prob->addVariable(var_name,
                        FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                               Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))),
                               &blocks);
  }
  else
  {
    prob->addAuxVariable(var_name,
                         FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                                Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))));
  }

  // Set initial condition
  Real initial = getParam<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol)
    _parser_handle._problem->addInitialCondition(var_name, initial);

#if 0
  Uncomment when adding Scaling
  if (is_variables_block) 
  {
    // Manual Scaling
    unsigned int var_number= system->variable_number(var_name);
    _moose_system._manual_scaling.push_back(getParam<Real>("scaling"));
    // This variable number should go in the same vector position as the manual scaling vector
    libmesh_assert(var_number == _moose_system._manual_scaling.size()-1);
  }
#endif

  // retrieve initial conditions from exodus file
  _variable_to_read = getParam<std::string>("initial_from_file_var");
  _timestep_to_read = getParam<int>("initial_from_file_timestep");
}

bool
AddVariableAction::restartRequired() const
{
  if (getParam<std::string>("initial_from_file_var") == "") 
    return false;
  else 
    return true;
}

bool
AddVariableAction::autoResizeable() const
{
  if (getParam<std::string>("order") == "FIRST" && getParam<std::string>("family") == "LAGRANGE")
    return true;
  else
    return false;
}


std::pair<std::string, unsigned int>
AddVariableAction::initialValuePair() const
{
  return std::pair<std::string, unsigned int>(_variable_to_read, _timestep_to_read);
}
