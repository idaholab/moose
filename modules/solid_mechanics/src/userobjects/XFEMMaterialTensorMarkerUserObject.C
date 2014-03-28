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

#include "XFEMMaterialTensorMarkerUserObject.h"


template<>
InputParameters validParams<XFEMMaterialTensorMarkerUserObject>()
{
  InputParameters params = validParams<XFEMMarkerUserObject>();
  params += validParams<MaterialTensorCalculator>();
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addRequiredParam<Real>("threshold", "The threshold for crack growth.");
  return params;
}

XFEMMaterialTensorMarkerUserObject::XFEMMaterialTensorMarkerUserObject(const std::string & name, InputParameters parameters):
  XFEMMarkerUserObject(name, parameters),
  _material_tensor_calculator(name, parameters),
  _tensor(getMaterialProperty<SymmTensor>(getParam<std::string>("tensor"))),
  _threshold(getParam<Real>("threshold"))
{
}


bool
XFEMMaterialTensorMarkerUserObject::doesElementCrack(RealVectorValue &direction)
{
  direction(1) = 1.0;
  return true;
}
