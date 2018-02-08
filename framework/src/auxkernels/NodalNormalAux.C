//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalAux.h"
#include "NodalNormalsUserObject.h"

template <>
InputParameters
validParams<NodalNormalAux>()
{
  InputParameters params = validParams<AuxKernel>();
  MooseEnum comp("X Y Z");
  params.addRequiredParam<MooseEnum>("component", comp, "The component of the normal");
  params.addRequiredParam<UserObjectName>("nodal_normals",
                                          "The name of the user object holding the nodal normals.");
  params.addClassDescription("This class is useful for visualization of the nodal normals.  It "
                             "takes a nodal normals user object and a component to be plotted. "
                             "Users need to set up one first order Lagrange variable per component "
                             "and add one instance of this kernel per component.");
  return params;
}

NodalNormalAux::NodalNormalAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _component_enum(getParam<MooseEnum>("component")),
    _component(_component_enum),
    _nodal_normals(getUserObject<NodalNormalsUserObject>("nodal_normals"))
{
  if (!isNodal())
    mooseError(name(),
               ": Your variable is not nodal - nodal normals can operate only on nodal variables.");
}

Real
NodalNormalAux::computeValue()
{
  return _nodal_normals.getNormal(_current_node->id())(_component);
}
