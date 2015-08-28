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

#include "RealFunctionControlMaterial.h"

template<>
InputParameters validParams<RealFunctionControlMaterial>()
{
  InputParameters params = validParams<ControlMaterial>();
  params.addParam<FunctionName>("function", "The name of the function to evaluate for the material property");
  params.addParam<MaterialPropertyName>("property", "The name of property to control");
  return params;
}

RealFunctionControlMaterial::RealFunctionControlMaterial(const InputParameters & parameters) :
    ControlMaterial(parameters),
    _function(getFunction("function")),
    _control_prop(getControlMaterialProperty<Real>("property"))
{
}

void
RealFunctionControlMaterial::computeQpProperties()
{
  _control_prop[_qp] = _function.value(_t, _q_point[_qp]);
}
