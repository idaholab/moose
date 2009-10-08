#include "GenericVariableBlock.h"

// libMesh includes
#include "libmesh.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "string_to_enum.h"

GenericVariableBlock::GenericVariableBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, input_file)
{
  _block_params.set<std::string>("family") = "LAGRANGE";
  _block_params.set<std::string>("order") = "FIRST";
}

void
GenericVariableBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericVariableBlock Object\n";
#endif

  TransientNonlinearImplicitSystem &system =
    Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

#ifdef DEBUG
  std::cerr << "Variable: " << getShortName()
            << "\torder: " << _block_params.get<std::string>("order")
            << "\tfamily: " << _block_params.get<std::string>("family") << std::endl;
#endif
  
  system.add_variable(getShortName(),
                      Utility::string_to_enum<Order>(_block_params.get<std::string>("order")),
                      Utility::string_to_enum<FEFamily>(_block_params.get<std::string>("family")));
  
}
