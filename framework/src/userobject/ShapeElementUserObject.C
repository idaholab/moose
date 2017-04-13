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

#include "ShapeElementUserObject.h"

template <>
InputParameters
validParams<ShapeElementUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params += ShapeUserObject<ElementUserObject>::validParams();
  return params;
}

ShapeElementUserObject::ShapeElementUserObject(const InputParameters & parameters)
  : ShapeUserObject<ElementUserObject>(parameters, ShapeType::Element)
{
}
