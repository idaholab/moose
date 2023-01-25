//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFEFluidWallMomentumBC.h"

registerMooseObject("NavierStokesApp", INSFEFluidWallMomentumBC);
registerMooseObjectRenamed("NavierStokesApp",
                           FluidWallMomentumBC,
                           "02/01/2024 00:00",
                           INSFEFluidWallMomentumBC);

InputParameters
INSFEFluidWallMomentumBC::validParams()
{
  InputParameters params = INSFEFluidIntegratedBCBase::validParams();
  params.addClassDescription("Implicitly sets normal component of velocity to zero if the "
                             "advection term of the momentum equation is integrated by parts");
  params.addRequiredParam<unsigned>("component", "the velocity component");
  return params;
}

INSFEFluidWallMomentumBC::INSFEFluidWallMomentumBC(const InputParameters & parameters)
  : INSFEFluidIntegratedBCBase(parameters),
    _mu(getMaterialProperty<Real>("dynamic_viscosity")),
    _mu_t(getMaterialProperty<Real>("turbulence_viscosity")),
    _component(getParam<unsigned>("component"))
{
}

Real
INSFEFluidWallMomentumBC::computeQpResidual()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  Real tau_w = (porosity > 0.99)
                   ? -(_mu[_qp] + _mu_t[_qp]) * _grad_u[_qp](_component) * _normals[_qp](_component)
                   : 0;

  return (porosity * _pressure[_qp] * _normals[_qp](_component) + tau_w) * _test[_i][_qp];
}

Real
INSFEFluidWallMomentumBC::computeQpJacobian()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  Real jac = (porosity > 0.99) ? -(_mu[_qp] + _mu_t[_qp]) * _grad_phi[_j][_qp](_component) *
                                     _normals[_qp](_component) * _test[_i][_qp]
                               : 0;

  return jac;
}

Real
INSFEFluidWallMomentumBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _pressure_var_number)
  {
    Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
    return porosity * _phi[_j][_qp] * _normals[_qp](_component) * _test[_i][_qp];
  }
  else
    return 0.0;
}
