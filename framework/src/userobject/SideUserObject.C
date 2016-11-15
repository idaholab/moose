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

#include "SideUserObject.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

template<>
InputParameters validParams<SideUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<BoundaryRestrictableRequired>();
  params += validParams<MaterialPropertyInterface>();
  return params;
}

SideUserObject::SideUserObject(const InputParameters & parameters) :
    UserObject(parameters),
    BoundaryRestrictableRequired(parameters, false), // false for applying to sidesets
    MaterialPropertyInterface(this, boundaryIDs()),
    Coupleable(this, false),
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
    _current_side_volume(_assembly.sideElemVolume())
{
  // Keep track of which variables are coupled so we know what we depend on
  addMooseVariableDependency(getCoupledMooseVars());
}
