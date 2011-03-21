#include "GenericVariableBlock.h"
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


template<>
InputParameters validParams<GenericVariableBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<std::string>("family", "LAGRANGE", "Specifies the family of FE shape functions to use for this variable");
  params.addParam<std::string>("order", "FIRST",  "Specifies the order of the FE shape function to use for this variable");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParam<int>("initial_from_file_timestep", 2, "Gives the timestep for which to read a solution from a file for a given variable");
  params.addParam<std::string>("initial_from_file_var", "Gives the name of a variable for which to read an initial condition from a mesh file");
  params.addParam<std::vector<unsigned int> >("block", "The block id where this variable lives");

  return params;
}


const Real GenericVariableBlock::_abs_zero_tol = 1e-12;

GenericVariableBlock::GenericVariableBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params),
  _variable_to_read(""),
  _timestep_to_read(2)
{
}

void
GenericVariableBlock::execute() 
{
  std::string var_name = getShortName();
  bool is_variables_block;

#ifdef DEBUG
  std::cerr << "Inside the GenericVariableBlock Object\n";
  std::cerr << "Variable: " << var_name
            << "\torder:  " << getParamValue<std::string>("order")
            << "\tfamily: " << getParamValue<std::string>("family") << std::endl;
#endif

  MProblem *prob = _parser_handle._problem;

  is_variables_block = Parser::pathContains(_name, "Variables");
  if (is_variables_block)
  {

    std::set<subdomain_id_type> blocks;
    std::vector<unsigned int> block_param = getParamValue<std::vector<unsigned int> >("block");
    for (std::vector<unsigned int>::iterator it = block_param.begin(); it != block_param.end(); ++it)
      blocks.insert(*it);
    Real scale_factor = getParamValue<Real>("scaling");

    if (blocks.empty())
	{
      prob->addVariable(var_name,
                        FEType(Utility::string_to_enum<Order>(getParamValue<std::string>("order")),
                               Utility::string_to_enum<FEFamily>(getParamValue<std::string>("family"))),
                        scale_factor);
	}
	else
      prob->addVariable(var_name,
                        FEType(Utility::string_to_enum<Order>(getParamValue<std::string>("order")),
                               Utility::string_to_enum<FEFamily>(getParamValue<std::string>("family"))),
                        scale_factor,
                        &blocks);
  }
  else
  {
    prob->addAuxVariable(var_name,
                         FEType(Utility::string_to_enum<Order>(getParamValue<std::string>("order")),
                                Utility::string_to_enum<FEFamily>(getParamValue<std::string>("family"))));
  }

  // Set initial condition
  Real initial = getParamValue<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol)
    _parser_handle._problem->addInitialCondition(var_name, initial);

  // retrieve initial conditions from exodus file
  _variable_to_read = getParamValue<std::string>("initial_from_file_var");
  _timestep_to_read = getParamValue<int>("initial_from_file_timestep");

  visitChildren();
}

bool
GenericVariableBlock::restartRequired() const
{
  if (getParamValue<std::string>("initial_from_file_var") == "") 
    return false;
  else 
    return true;
}

bool
GenericVariableBlock::autoResizeable() const
{
  if (getParamValue<std::string>("order") == "FIRST" && getParamValue<std::string>("family") == "LAGRANGE")
    return true;
  else
    return false;
}


std::pair<std::string, unsigned int>
GenericVariableBlock::initialValuePair() const
{
  return std::pair<std::string, unsigned int>(_variable_to_read, _timestep_to_read);
}

