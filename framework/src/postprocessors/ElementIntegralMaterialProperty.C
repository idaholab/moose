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

#include "ElementIntegralMaterialProperty.h"

template <>
InputParameters
validParams<ElementIntegralMaterialProperty>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "The name of the material property");
  return params;
}

ElementIntegralMaterialProperty::ElementIntegralMaterialProperty(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters), _scalar(getMaterialProperty<Real>("mat_prop"))
{
}

Real
ElementIntegralMaterialProperty::computeQpIntegral()
{
  return _scalar[_qp];
}
