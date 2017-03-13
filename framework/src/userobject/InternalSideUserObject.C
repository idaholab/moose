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

#include "InternalSideUserObject.h"
#include "Assembly.h"

template<>
InputParameters validParams<InternalSideUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<TwoMaterialPropertyInterface>();
  return params;
}

InternalSideUserObject::InternalSideUserObject(const InputParameters & parameters) :
    UserObject(parameters),
    BlockRestrictable(parameters),
    TwoMaterialPropertyInterface(this, blockIDs()),
    NeighborCoupleable(this, false, false),
    MooseVariableDependencyInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    ZeroInterface(parameters),
    _mesh(_subproblem.mesh()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _current_elem(_assembly.elem()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _neighbor_elem(_assembly.neighbor())
{
  // Keep track of which variables are coupled so we know what we depend on
  addMooseVariableDependency(getCoupledMooseVars());
}


const Real &
InternalSideUserObject::getNeighborElemVolume()
{
  return _assembly.neighborVolume();
}
