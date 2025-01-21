//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriInterWrapperBaseIC.h"
#include "TriInterWrapperMesh.h"

InputParameters
TriInterWrapperBaseIC::validParams()
{
  return InitialCondition::validParams();
}

TriInterWrapperBaseIC::TriInterWrapperBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(getMesh(_fe_problem.mesh()))
{
}

TriInterWrapperMesh &
TriInterWrapperBaseIC::getMesh(MooseMesh & mesh)
{
  TriInterWrapperMesh * m = dynamic_cast<TriInterWrapperMesh *>(&mesh);
  if (m)
    return dynamic_cast<TriInterWrapperMesh &>(mesh);
  else
    mooseError(name(),
               ": This initial condition works only with triangular subchannel geometry. Update "
               "your input file to use TriInterWrapperMesh in the mesh block.");
}
