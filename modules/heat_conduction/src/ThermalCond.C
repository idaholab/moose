/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ThermalCond.h"

template<>
InputParameters validParams<ThermalCond>()
{
  InputParameters params = validParams<SideAverageValue>();
  params.addRequiredParam<Real>("dx","Length between sides of sample in length_scale");
  params.addRequiredParam<Real>("flux","Heat flux out of 'cold' boundary in solution units, should always be positive");
  params.addRequiredParam<Real>("T_hot","Temperature on 'hot' boundary in K");
  params.addParam<Real>("length_scale",1e-8,"lengthscale of the solution, default is 1e-8");
  params.addParam<Real>("k0",0.0,"Initial value of the thermal conductivity");

  return params;
}

ThermalCond::ThermalCond(const std::string & name, InputParameters parameters)
  :SideAverageValue(name, parameters),
   _dx(getParam<Real>("dx")),
   _flux(getParam<Real>("flux")),
   _T_hot(getParam<Real>("T_hot")),
   _length_scale(getParam<Real>("length_scale")),
   _k0(getParam<Real>("k0"))
{}

Real
ThermalCond::getValue()
{
  Real T_cold = SideAverageValue::getValue();

  Real Th_cond = 0.0;


  if (std::abs(_T_hot - T_cold) > 1.0e-20)
    Th_cond = _flux*_dx/std::abs(_T_hot-T_cold); //Calculate effective thermal conductivity in W/(length_scale-K)

  if (_t_step == 0)
    return _k0;
  else
    return Th_cond/_length_scale; //In W/(m-K)

}
