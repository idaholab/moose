/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "Q2PMass.h"

template<>
InputParameters validParams<Q2PMass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addRequiredParam<UserObjectName>("fluid_density", "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredCoupledVar("other_var", "If the variable is set to the saturation then mass = porosity*density*saturation, and other_var must be set to the porepressure.  If the variable is set to porepressure then mass = porosity*density*(1-saturation), and other_var must e set to saturation.");
    params.addRequiredParam<bool>("var_is_porepressure", "Set to true if the variable = porepressure.  Otherwise set false");
  params.addClassDescription("Returns the mass in a region.");
  return params;
}

Q2PMass::Q2PMass(const InputParameters & parameters) :
    ElementIntegralVariablePostprocessor(parameters),
    _porosity(getMaterialProperty<Real>("porosity")),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _other_var(coupledValue("other_var")),
    _var_is_pp(getParam<bool>("var_is_porepressure"))
{
}

Real
Q2PMass::computeQpIntegral()
{
  if (_var_is_pp)
    return _porosity[_qp]*_density.density(_u[_qp])*(1 - _other_var[_qp]);

  return _porosity[_qp]*_density.density(_other_var[_qp])*_u[_qp];
}

