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

#include "BCsBlock.h"

#include "BCFactory.h"

template<>
InputParameters validParams<BCsBlock>()
{
  return validParams<ParserBlock>();
}

BCsBlock::BCsBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register BCs/AuxBCs prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
  addPrereq("AuxKernels");
}

void
BCsBlock::execute() 
{
#ifdef DEBUG
  if (_parser_handle.pathContains(_name, "BCs"))
    std::cerr << "Inside the BCsBlock Object\n";
  else
    std::cerr << "Inside the BCsBlock (Aux) Object\n";
#endif

//  if (_reg_id == "BCs")
//    BoundaryCondition::init();
  
  // Add the BCs or AuxBCs to the system
  visitChildren();
}
