/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

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
