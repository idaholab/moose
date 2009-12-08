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

GenericVariableBlock::GenericVariableBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _variable_to_read(""),
   _timestep_to_read(2)
{}

void
GenericVariableBlock::execute() 
{
  std::string var_name = getShortName();
  
#ifdef DEBUG
  std::cerr << "Inside the GenericVariableBlock Object\n";
  std::cerr << "Variable: " << var_name
            << "\torder: " << getParamValue<std::string>("order")
            << "\tfamily: " << getParamValue<std::string>("family") << std::endl;
#endif

  System *system;
  if (_reg_id == "Variables/*")
    system = &Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  else
    system = &Moose::equation_system->get_system<TransientExplicitSystem>("AuxiliarySystem");

  system->add_variable(var_name,
                       Utility::string_to_enum<Order>(getParamValue<std::string>("order")),
                       Utility::string_to_enum<FEFamily>(getParamValue<std::string>("family")));
  
  // Set initial condition
  Real initial = getParamValue<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol) 
    Moose::equation_system->parameters.set<Real>("initial_" + var_name) = initial;

  
  if (_reg_id == "Variables/*") 
  {
    // Manual Scaling
    unsigned int var_number= system->variable_number(var_name);
    Moose::manual_scaling.push_back(getParamValue<Real>("scaling"));
    // This variable number should go in the same vector position as the manual scaling vector
    libmesh_assert(var_number == Moose::manual_scaling.size()-1);
  }

  // retrieve inital conditions from exodus file
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
