//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidWallMomentumBC.h"

registerMooseObject("NavierStokesApp", FluidWallMomentumBC);

InputParameters
FluidWallMomentumBC::validParams()
{
  InputParameters params = MDFluidIntegratedBCBase::validParams();
  params.addRequiredParam<unsigned>("component", "the velocity component");
  return params;
}

FluidWallMomentumBC::FluidWallMomentumBC(const InputParameters & parameters)
  : MDFluidIntegratedBCBase(parameters),
    _mu(getMaterialProperty<Real>("dynamic_viscosity")),
    _mu_t(getMaterialProperty<Real>("turbulence_viscosity")),
    _component(getParam<unsigned>("component"))
{
}

Real
FluidWallMomentumBC::computeQpResidual()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  Real tau_w = (porosity > 0.99)
                   ? -(_mu[_qp] + _mu_t[_qp]) * _grad_u[_qp](_component) * _normals[_qp](_component)
                   : 0;

  return (porosity * _pressure[_qp] * _normals[_qp](_component) + tau_w) * _test[_i][_qp];
}

Real
FluidWallMomentumBC::computeQpJacobian()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  Real jac = (porosity > 0.99) ? -(_mu[_qp] + _mu_t[_qp]) * _grad_phi[_j][_qp](_component) *
                                     _normals[_qp](_component) * _test[_i][_qp]
                               : 0;

  return jac;
}

Real
FluidWallMomentumBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _pressure_var_number)
  {
    Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
    return porosity * _phi[_j][_qp] * _normals[_qp](_component) * _test[_i][_qp];
  }
  else
    return 0.0;
}
