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

#include "GenericVariableBlock.h"
#include "Moose.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"
#include "string_to_enum.h"

const Real GenericVariableBlock::_abs_zero_tol = 1e-12;

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

GenericVariableBlock::GenericVariableBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _variable_to_read(""),
   _timestep_to_read(2)
{}

void
GenericVariableBlock::execute() 
{
  std::string var_name = getShortName();
  bool is_variables_block;
  
#ifdef DEBUG
  std::cerr << "Inside the GenericVariableBlock Object\n";
  std::cerr << "Variable: " << var_name
            << "\torder: " << getParamValue<std::string>("order")
            << "\tfamily: " << getParamValue<std::string>("family") << std::endl;
#endif

  System *system;
  is_variables_block = Parser::pathContains(_name, "Variables");
  if (is_variables_block)
  {
    system = _moose_system.getNonlinearSystem();

    std::set<subdomain_id_type> blocks;
    std::vector<unsigned int> block_param = getParamValue<std::vector<unsigned int> >("block");
    for (std::vector<unsigned int>::iterator it = block_param.begin(); it != block_param.end(); ++it)
      blocks.insert(*it);

    if (blocks.empty())
      _moose_system.addVariable(var_name,
                                Utility::string_to_enum<Order>(getParamValue<std::string>("order")),
                                Utility::string_to_enum<FEFamily>(getParamValue<std::string>("family")));
    else
      _moose_system.addVariable(var_name,
                                Utility::string_to_enum<Order>(getParamValue<std::string>("order")),
                                Utility::string_to_enum<FEFamily>(getParamValue<std::string>("family")), &blocks);
  }
  else
  {
    system = _moose_system.getAuxSystem();

    _moose_system.addAuxVariable(var_name,
                                 Utility::string_to_enum<Order>(getParamValue<std::string>("order")),
                                 Utility::string_to_enum<FEFamily>(getParamValue<std::string>("family")));
  }
  
  
  // Set initial condition
  Real initial = getParamValue<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol) 
    _moose_system.getEquationSystems()->parameters.set<Real>("initial_" + var_name) = initial;

  
  if (is_variables_block) 
  {
    // Manual Scaling
    unsigned int var_number= system->variable_number(var_name);
    _moose_system._manual_scaling.push_back(getParamValue<Real>("scaling"));
    // This variable number should go in the same vector position as the manual scaling vector
    libmesh_assert(var_number == _moose_system._manual_scaling.size()-1);
  }

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
