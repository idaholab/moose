/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Q2PNodalMass.h"

#include <iostream>

template <>
InputParameters
validParams<Q2PNodalMass>()
{
  InputParameters params = validParams<TimeKernel>();
  params.addRequiredParam<UserObjectName>(
      "fluid_density",
      "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredCoupledVar("other_var",
                               "The other variable in the 2-phase system.  If "
                               "Variable=porepressure, then other_var should be the "
                               "saturation Variable, and vice-versa.");
  params.addRequiredParam<bool>(
      "var_is_porepressure",
      "This flag is needed to correctly calculate the Jacobian entries.  If "
      "set to true, this Kernel will assume it is describing the mass of "
      "the phase with porepressure as its Variable (eg, the liquid phase).  "
      "If set to false, this Kernel will assumed it is describing the mass "
      "of the phase with saturation as its variable (eg, the gas phase)");
  params.addClassDescription("Fluid mass lumped to the nodes divided by dt");
  return params;
}

Q2PNodalMass::Q2PNodalMass(const InputParameters & parameters)
  : TimeKernel(parameters),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _other_var_nodal(coupledNodalValue("other_var")),
    _other_var_num(coupled("other_var")),
    _var_is_pp(getParam<bool>("var_is_porepressure")),
    _porosity(getMaterialProperty<Real>("porosity"))
{
}

Real
Q2PNodalMass::computeQpResidual()
{
  Real density;
  Real mass;

  if (_var_is_pp)
  {
    density = _density.density(_var.nodalSln()[_i]);
    mass = _porosity[_qp] * density * (1 - _other_var_nodal[_i]);
  }
  else
  {
    density = _density.density(_other_var_nodal[_i]);
    mass = _porosity[_qp] * density * _var.nodalSln()[_i];
  }

  return _test[_i][_qp] * mass / _dt;
}

Real
Q2PNodalMass::computeQpJacobian()
{
  if (_i != _j)
    return 0.0;

  Real mass_prime;

  if (_var_is_pp)
  {
    // we're calculating the derivative wrt porepressure
    Real ddensity = _density.ddensity(_var.nodalSln()[_i]);
    mass_prime = _porosity[_qp] * ddensity * (1 - _other_var_nodal[_i]);
  }
  else
  {
    // we're calculating the deriv wrt saturation
    Real density = _density.density(_other_var_nodal[_i]);
    mass_prime = _porosity[_qp] * density;
  }

  return _test[_i][_qp] * mass_prime / _dt;
}

Real
Q2PNodalMass::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _other_var_num)
    return 0.0;
  if (_i != _j)
    return 0.0;

  Real mass_prime;

  if (_var_is_pp)
  {
    // we're calculating the deriv wrt saturation
    Real density = _density.density(_var.nodalSln()[_i]);
    mass_prime = -_porosity[_qp] * density;
  }
  else
  {
    // we're calculating the deriv wrt porepressure
    Real ddensity = _density.ddensity(_other_var_nodal[_i]);
    mass_prime = _porosity[_qp] * ddensity * _var.nodalSln()[_i];
  }

  return _test[_i][_qp] * mass_prime / _dt;
}
