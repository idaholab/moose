//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMassAdvection.h"

registerMooseObject("NavierStokesTestApp", ADMassAdvection);

InputParameters
ADMassAdvection::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("vel_x", "The x-component of velocity");
  params.addCoupledVar("vel_y", "The y-component of velocity");
  params.addCoupledVar("vel_z", "The z-component of velocity");
  return params;
}

ADMassAdvection::ADMassAdvection(const InputParameters & parameters)
  : ADKernel(parameters),
    _vel_x(adCoupledValue("vel_x")),
    _vel_y(isCoupled("vel_y") ? adCoupledValue("vel_y") : _ad_zero),
    _vel_z(isCoupled("vel_z") ? adCoupledValue("vel_z") : _ad_zero),
    _grad_vel_x(adCoupledGradient("vel_x")),
    _grad_vel_y(isCoupled("vel_y") ? adCoupledGradient("vel_y") : _ad_grad_zero),
    _grad_vel_z(isCoupled("vel_z") ? adCoupledGradient("vel_z") : _ad_grad_zero),
    _coord_sys(_assembly.coordSystem()),
    _rz_radial_coord(_mesh.getAxisymmetricRadialCoord())
{
}

ADReal
ADMassAdvection::computeQpResidual()
{
  // (div u) * q
  // Note: we (arbitrarily) multiply this term by -1 so that it matches the -p(div v)
  // term in the momentum equation.  Not sure if that is really important?
  auto residual =
      -(_grad_vel_x[_qp](0) + _grad_vel_y[_qp](1) + _grad_vel_z[_qp](2)) * _test[_i][_qp];
  if (_coord_sys == Moose::COORD_RZ)
    // Subtract u_r / r
    residual -=
        [this]()
    {
      switch (_rz_radial_coord)
      {
        case 0:
          return _vel_x[_qp];
        case 1:
          return _vel_y[_qp];
        default:
          mooseError("Unexpected radial coordinate");
      }
    }() /
        _ad_q_point[_qp](_rz_radial_coord) * _test[_i][_qp];
  return residual;
}
