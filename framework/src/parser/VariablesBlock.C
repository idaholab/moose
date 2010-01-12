#include "VariablesBlock.h"

// Moose includes
#include "Moose.h"
#include "ComputeResidual.h"
#include "ComputeJacobian.h"
#include "AuxFactory.h"
#include "GenericVariableBlock.h"

// libMesh includes
#include "libmesh.h"
#include "mesh.h"
#include "equation_systems.h"
#include "nonlinear_solver.h"
#include "nonlinear_implicit_system.h"
#include "getpot.h"
#include "exodusII_io.h"

template<>
InputParameters validParams<VariablesBlock>()
{
  return validParams<ParserBlock>();
}

VariablesBlock::VariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
}

void
VariablesBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the VariablesBlock Object\n";
#endif

  EquationSystems *equation_system = new EquationSystems(*Moose::mesh);
  Moose::equation_system = equation_system;
  TransientNonlinearImplicitSystem &system =
    equation_system->add_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

  Moose::manual_scaling.reserve(n_activeChildren());

  // Add variable blocks from the children nodes
  visitChildren();

  system.nonlinear_solver->residual = Moose::compute_residual;
  system.nonlinear_solver->jacobian = Moose::compute_jacobian;

  system.attach_init_function(Moose::init_cond);

  /** If requested, nodal values are copied out after the equation systems are initialized.
   *  This call is made from the AuxVariables Block
   */
}

void
VariablesBlock::copyNodalValues(const std::string &system_name)
{
  System *system;
  if (system_name == "NonlinearSystem")
    system = &Moose::equation_system->get_system<TransientNonlinearImplicitSystem>(system_name);
  else
    system = &Moose::equation_system->get_system<TransientExplicitSystem>(system_name);
  
  // Iterate over the children and see if they need nodal values read
  for (std::vector<ParserBlock *>::iterator i = _children.begin(); i!=_children.end(); ++i)
  {
    if (GenericVariableBlock * var_block = dynamic_cast<GenericVariableBlock *>(*i))
    {
      std::pair<std::string, unsigned int> init_pair = var_block->initialValuePair();
      if (init_pair.first != "") 
        Moose::exreader->copy_nodal_solution(*system, init_pair.first, init_pair.second);
    }
  }
}
