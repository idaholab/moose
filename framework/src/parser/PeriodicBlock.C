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

#include "PeriodicBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

#include "dof_map.h"
#include "mesh_refinement.h"

template<>
InputParameters validParams<PeriodicBlock>()
{
  return validParams<ParserBlock>();
}

PeriodicBlock::PeriodicBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params),
   _executed(false)
{}

void
PeriodicBlock::execute() 
{
  // If this block has already been executed once... don't do it again.
  if(_executed)
    return;

  _executed = true;

  visitChildren();

  // Periodic Boundaries have been added so make the MeshRefinement object aware of them
  MeshRefinement &r = _moose_system.getMeshRefinementObject();
  r.set_periodic_boundaries_ptr(_moose_system.getNonlinearSystem()->get_dof_map().get_periodic_boundaries());
}  
