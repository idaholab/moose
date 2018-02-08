//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalBC.h"
#include "NodalNormalsUserObject.h"

template <>
InputParameters
validParams<NodalNormalBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredParam<UserObjectName>("nodal_normals",
                                          "The user object holding the nodal normals.");
  return params;
}

NodalNormalBC::NodalNormalBC(const InputParameters & parameters)
  : NodalBC(parameters), _nodal_normals(getUserObject<NodalNormalsUserObject>("nodal_normals"))
{
}

void
NodalNormalBC::computeResidual(NumericVector<Number> & residual)
{
  _qp = 0;
  _normal = _nodal_normals.getNormal(_current_node->id());
  NodalBC::computeResidual(residual);
}
