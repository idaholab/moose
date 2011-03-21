#include "VariablesBlock.h"

// Moose includes
#include "Moose.h"
#include "GenericVariableBlock.h"
#include "SystemBase.h"
#include "Parser.h"

// libMesh includes
#include "libmesh.h"
#include "MooseMesh.h"
#include "equation_systems.h"
#include "nonlinear_solver.h"
#include "nonlinear_implicit_system.h"
#include "getpot.h"
#include "exodusII_io.h"
#include "dof_map.h"

template<>
InputParameters validParams<VariablesBlock>()
{
  return validParams<ParserBlock>();
}


VariablesBlock::VariablesBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _cm(NULL)
{
  if (!_parser_handle._loose)
    addPrereq("Executioner");
#if 0
  // Register execution prereqs
  addPrereq("Mesh");
#endif
}

VariablesBlock::~VariablesBlock()
{
  delete _cm;
}

void
VariablesBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the VariablesBlock Object\n";
#endif

  if (!_executed)
  {
/*
    _moose_system.initEquationSystems();
    _moose_system.initDataStructures();

    TransientNonlinearImplicitSystem &system = *_moose_system.getNonlinearSystem();

*/

    // Add variable blocks from the children nodes
    visitChildren();

/*
    // FIXME: should be inside MooseSystem
    _cm = new CouplingMatrix(system.n_vars());

    for(unsigned int i=0; i<system.n_vars(); i++)
      for(unsigned int j=0; j<system.n_vars(); j++)
        (*_cm)(i, j) = ( i == j ? 1 : 0);
  
    system.get_dof_map()._dof_coupling = _cm;
*/

    /** If requested, nodal values are copied out after the equation systems are initialized.
     *  This call is made from the AuxVariables Block
     */

    _executed = true;
  }
}

void
VariablesBlock::copyNodalValues(SystemBase & sys)
{
  // Iterate over the children and see if they need nodal values read
  for (std::vector<ParserBlock *>::iterator i = _children.begin(); i!=_children.end(); ++i)
  {
    if (GenericVariableBlock * var_block = dynamic_cast<GenericVariableBlock *>(*i))
    {
      std::pair<std::string, unsigned int> init_pair = var_block->initialValuePair();
      if (init_pair.first != "")
        sys.copyNodalValues(*_parser_handle._exreader, init_pair.first, init_pair.second);
    }
  }
}

