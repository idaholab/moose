//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriSubChannelBaseIC.h"
#include "TriSubChannelMesh.h"

InputParameters
TriSubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

TriSubChannelBaseIC::TriSubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(getMesh(_fe_problem.mesh()))
{
}

TriSubChannelMesh &
TriSubChannelBaseIC::getMesh(MooseMesh & mesh)
{
  TriSubChannelMesh * m = dynamic_cast<TriSubChannelMesh *>(&mesh);
  if (m)
    return dynamic_cast<TriSubChannelMesh &>(mesh);
  else
    mooseError(name(),
               ": This initial condition works only with triangular subchannel geometry. Update "
               "your input file to use TriSubChannelMesh in the mesh block.");
}
