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

#include "MatSideUserObject.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MatSideUserObject>()
{
  InputParameters params = validParams<SideUserObject>();
  params.addRequiredParam<MaterialPropertyName>(
      "mat_prop", "the name of the material property we are going to use");
  return params;
}

MatSideUserObject::MatSideUserObject(const InputParameters & parameters)
  : SideUserObject(parameters), _mat_prop(getMaterialProperty<Real>("mat_prop"))
{
}
