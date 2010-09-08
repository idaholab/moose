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

#include "VariablesBlock.h"

// Moose includes
#include "Moose.h"
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
#include "dof_map.h"
#include "coupling_matrix.h"

template<>
InputParameters validParams<VariablesBlock>()
{
  return validParams<ParserBlock>();
}

VariablesBlock::VariablesBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
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

  _moose_system.initEquationSystems();
  _moose_system.initDataStructures();

  TransientNonlinearImplicitSystem &system = *_moose_system.getNonlinearSystem();

  _moose_system._manual_scaling.reserve(n_activeChildren());

  // Add variable blocks from the children nodes
  visitChildren();

  // FIXME: should be inside MooseSystem
  CouplingMatrix * cm = new CouplingMatrix(system.n_vars());
  
  for(unsigned int i=0; i<system.n_vars(); i++)
    for(unsigned int j=0; j<system.n_vars(); j++)
      (*cm)(i, j) = ( i == j ? 1 : 0);

  system.get_dof_map()._dof_coupling = cm;

  /** If requested, nodal values are copied out after the equation systems are initialized.
   *  This call is made from the AuxVariables Block
   */
}

void
VariablesBlock::copyNodalValues(const std::string &system_name)
{
  System *system;
  if (system_name == "NonlinearSystem")
    system = _moose_system.getNonlinearSystem();
  else
    system = _moose_system.getAuxSystem();
  
  // Iterate over the children and see if they need nodal values read
  for (std::vector<ParserBlock *>::iterator i = _children.begin(); i!=_children.end(); ++i)
  {
    if (GenericVariableBlock * var_block = dynamic_cast<GenericVariableBlock *>(*i))
    {
      std::pair<std::string, unsigned int> init_pair = var_block->initialValuePair();
      if (init_pair.first != "") 
        _moose_system.getExodusReader()->copy_nodal_solution(*system, init_pair.first, init_pair.second);
    }
  }
}
