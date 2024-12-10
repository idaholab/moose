//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadInterWrapperBaseIC.h"
#include "QuadInterWrapperMesh.h"

InputParameters
QuadInterWrapperBaseIC::validParams()
{
  return InitialCondition::validParams();
}

QuadInterWrapperBaseIC::QuadInterWrapperBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(getMesh(_fe_problem.mesh()))
{
}

QuadInterWrapperMesh &
QuadInterWrapperBaseIC::getMesh(MooseMesh & mesh)
{
  QuadInterWrapperMesh * m = dynamic_cast<QuadInterWrapperMesh *>(&mesh);
  if (m)
    return dynamic_cast<QuadInterWrapperMesh &>(mesh);
  else
    mooseError(name(),
               ": This initial condition works only with quadrilateral mesh. Update your input "
               "file to use [QuadInterWrapperMesh] block for mesh.");
}
