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
