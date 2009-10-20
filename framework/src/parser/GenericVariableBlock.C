#include "GenericVariableBlock.h"

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

GenericVariableBlock::GenericVariableBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _variable_to_read(""),
   _timestep_to_read(2)
{
  _block_params.set<std::string>("family") = "LAGRANGE";
  _block_params.set<std::string>("order") = "FIRST";
  _block_params.set<Real>("initial_condition") = 0.0;
  _block_params.set<Real>("scaling") = 1.0;
  _block_params.set<std::string>("initial_from_file_var");
  _block_params.set<int>("initial_from_file_timestep");
}

void
GenericVariableBlock::execute() 
{
  std::string var_name = getShortName();
  
#ifdef DEBUG
  std::cerr << "Inside the GenericVariableBlock Object\n";
  std::cerr << "Variable: " << var_name
            << "\torder: " << _block_params.get<std::string>("order")
            << "\tfamily: " << _block_params.get<std::string>("family") << std::endl;
#endif

  System *system;
  if (_reg_id == "Variables/*")
    system = &Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  else
    system = &Moose::equation_system->get_system<TransientExplicitSystem>("AuxiliarySystem");

  system->add_variable(var_name,
                       Utility::string_to_enum<Order>(_block_params.get<std::string>("order")),
                       Utility::string_to_enum<FEFamily>(_block_params.get<std::string>("family")));
  
  // Set initial condition
  Real initial = _block_params.get<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol) 
    Moose::equation_system->parameters.set<Real>("initial_" + var_name) = initial;

  
  if (_reg_id == "Variables/*") 
  {
    // Manual Scaling
    unsigned int var_number= system->variable_number(var_name);
    Moose::manual_scaling.push_back(_block_params.get<Real>("scaling"));
    // This variable number should go in the same vector position as the manual scaling vector
    libmesh_assert(var_number == Moose::manual_scaling.size()-1);
  }

  // retrieve inital conditions from exodus file
  _variable_to_read = _block_params.get<std::string>("initial_from_file_var");
  _timestep_to_read = _block_params.get<int>("initial_from_file_timestep");  
}

bool
GenericVariableBlock::restartRequired() const
{
  if (_block_params.get<std::string>("initial_from_file_var") == "") 
    return false;
  else 
    return true;
}

std::pair<std::string, unsigned int>
GenericVariableBlock::initialValuePair() const
{
  return std::pair<std::string, unsigned int>(_variable_to_read, _timestep_to_read);
}
