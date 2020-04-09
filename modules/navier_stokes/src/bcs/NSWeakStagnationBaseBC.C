//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSWeakStagnationBaseBC.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// Full specialization of the validParams function for this object

InputParameters
NSWeakStagnationBaseBC::validParams()
{
  InputParameters params = NSIntegratedBC::validParams();
  params.addClassDescription(
      "This is the base class for 'weakly-imposed' stagnation boundary conditions.");
  params.addRequiredParam<Real>("stagnation_pressure", "The specifed stagnation pressure");
  params.addRequiredParam<Real>("stagnation_temperature", "The specifed stagnation temperature");
  params.addRequiredParam<Real>("sx", "x-component of specifed flow direction");
  params.addRequiredParam<Real>("sy", "y-component of specifed flow direction");
  params.addParam<Real>("sz", 0.0, "z-component of specifed flow direction"); // only required in 3D
  return params;
}

NSWeakStagnationBaseBC::NSWeakStagnationBaseBC(const InputParameters & parameters)
  : NSIntegratedBC(parameters),
    _stagnation_pressure(getParam<Real>("stagnation_pressure")),
    _stagnation_temperature(getParam<Real>("stagnation_temperature")),
    _sx(getParam<Real>("sx")),
    _sy(getParam<Real>("sy")),
    _sz(getParam<Real>("sz"))
{
}

void
NSWeakStagnationBaseBC::staticValues(Real & T_s, Real & p_s, Real & rho_s)
{
  // T_s = T_0 - |u|^2/2/cp
  T_s = _stagnation_temperature - 0.5 * this->velmag2() / _fp.cp();

  if (T_s < 0.)
    mooseError("Negative temperature detected in NSWeakStagnationBaseBC!");

  // p_s = p_0 * (T_0/T)^(-gam/(gam-1))
  p_s = _stagnation_pressure *
        std::pow(_stagnation_temperature / T_s, -_fp.gamma() / (_fp.gamma() - 1.));

  // Compute static rho from static pressure and temperature using equation of state.
  rho_s = _fp.rho_from_p_T(p_s, T_s);
}

Real
NSWeakStagnationBaseBC::rhoStatic()
{
  Real T_s = 0., p_s = 0., rho_s = 0.;
  staticValues(T_s, p_s, rho_s);
  return rho_s;
}

Real
NSWeakStagnationBaseBC::velmag2()
{
  return _u_vel[_qp] * _u_vel[_qp] + _v_vel[_qp] * _v_vel[_qp] + _w_vel[_qp] * _w_vel[_qp];
}

Real
NSWeakStagnationBaseBC::sdotn()
{
  return _sx * _normals[_qp](0) + _sy * _normals[_qp](1) + _sz * _normals[_qp](2);
}
