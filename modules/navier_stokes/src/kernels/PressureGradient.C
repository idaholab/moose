//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureGradient.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PressureGradient);

InputParameters
PressureGradient::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<unsigned int>("component", "number of component (0 = x, 1 = y, 2 = z)");
  params.addClassDescription(
      "Implements the pressure gradient term for one of the Navier Stokes momentum equations.");
  params.addParam<bool>(
      "integrate_p_by_parts", true, "Whether to integrate the pressure term by parts");

  return params;
}

PressureGradient::PressureGradient(const InputParameters & parameters)
  : Kernel(parameters),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts")),
    _component(getParam<unsigned int>("component")),
    _pressure(coupledValue(NS::pressure)),
    _grad_pressure(coupledGradient(NS::pressure)),
    _pressure_id(coupled(NS::pressure)),
    _coord_sys(_assembly.coordSystem()),
    _rz_radial_coord(_mesh.getAxisymmetricRadialCoord())
{
}

Real
PressureGradient::computeQpResidual()
{
  if (_integrate_p_by_parts)
  {
    auto residual = -_pressure[_qp] * _grad_test[_i][_qp](_component);
    if (_coord_sys == Moose::COORD_RZ && (_rz_radial_coord == _component))
      residual -= _pressure[_qp] / _q_point[_qp](_rz_radial_coord) * _test[_i][_qp];
    return residual;
  }
  else
    return _test[_i][_qp] * _grad_pressure[_qp](_component);
}

Real
PressureGradient::computeQpJacobian()
{
  return 0.;
}

Real
PressureGradient::computeQpOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _pressure_id)
  {
    if (_integrate_p_by_parts)
    {
      auto residual = -_phi[_j][_qp] * _grad_test[_i][_qp](_component);
      if (_coord_sys == Moose::COORD_RZ && (_rz_radial_coord == _component))
        residual -= _phi[_j][_qp] / _q_point[_qp](_rz_radial_coord) * _test[_i][_qp];
      return residual;
    }
    else
      return _test[_i][_qp] * _grad_phi[_j][_qp](_component);
  }
  else
    return 0;
}
