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

const Real GenericVariableBlock::_abs_zero_tol = 1e12;

GenericVariableBlock::GenericVariableBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _variable_to_read(""),
   _timestep_to_read(2)
{
  _block_params.set<std::string>("family") = "LAGRANGE";
  _block_params.set<std::string>("order") = "FIRST";
  _block_params.set<Real>("initial_condition") = 0.0;
  _block_params.set<Real>("scaling") = 0.0;
  _block_params.set<std::vector<std::string> >("initial_from_file");
}

void
GenericVariableBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericVariableBlock Object\n";
  std::cerr << "Variable: " << getShortName()
            << "\torder: " << _block_params.get<std::string>("order")
            << "\tfamily: " << _block_params.get<std::string>("family") << std::endl;
#endif

  System *system;
  if (_reg_id == "Variables/*")
    system = &Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  else
    system = &Moose::equation_system->get_system<TransientExplicitSystem>("AuxiliarySystem");

  system->add_variable(getShortName(),
                       Utility::string_to_enum<Order>(_block_params.get<std::string>("order")),
                       Utility::string_to_enum<FEFamily>(_block_params.get<std::string>("family")));
  
  // Set initial condition
  Real initial = _block_params.get<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol)
    Moose::equation_system->parameters.set<Real>("initial_" + getShortName()) = initial;
  
  // retrieve inital conditions from exodus file
  std::vector<std::string> initial_from_file = _block_params.get<std::vector<std::string> >("initial_from_file");
  if (initial_from_file.size()) 
  {
    _variable_to_read = initial_from_file.at(0);
    _timestep_to_read = 2;

    // TODO: WTF?
    /*try 
    {
      std::stringstream iss(initial_from_file.at(1));
      if ((iss >> _timestep_to_read).fail())
        libmesh_error();
    }
    catch (std::out_of_range & e)
    {
      // default to reading timestep 2 (which will be the final solution from a steady state calc
      _timestep_to_read = 2;
      }*/
  }
  
  // TODO: Manual Scaling
  
}

bool
GenericVariableBlock::restartRequired() const
{
  return bool(_block_params.get<std::vector<std::string> >("initial_from_file").size());
}

std::pair<std::string, unsigned int>
GenericVariableBlock::initialValuePair() const
{
  return std::pair<std::string, unsigned int>(_variable_to_read, _timestep_to_read);
}
