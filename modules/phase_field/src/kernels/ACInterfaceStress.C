//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACInterfaceStress.h"
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"

registerMooseObject("PhaseFieldApp", ACInterfaceStress);

InputParameters
ACInterfaceStress::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Interface stress driving force Allen-Cahn Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<Real>("stress", "Planar stress");
  params.addRangeCheckedParam<Real>("op_range",
                                    1.0,
                                    "op_range > 0.0",
                                    "Range over which order parameters change across an "
                                    "interface. By default order parameters are assumed to "
                                    "vary from 0 to 1");
  return params;
}

ACInterfaceStress::ACInterfaceStress(const InputParameters & parameters)
  : Kernel(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "elastic_strain")),
    _stress(getParam<Real>("stress") / getParam<Real>("op_range"))
{
}

Real
ACInterfaceStress::computeQpResidual()
{
  // no interface, return zero stress
  const Real grad_norm_sq = _grad_u[_qp].norm_sq();
  if (grad_norm_sq < libMesh::TOLERANCE)
    return 0.0;

  const Real grad_norm = std::sqrt(grad_norm_sq);

  const Real nx = _grad_u[_qp](0);
  const Real ny = _grad_u[_qp](1);
  const Real nz = _grad_u[_qp](2);

  const Real s = _stress / grad_norm;
  const Real ds = -_stress / (grad_norm * grad_norm_sq);
  const Real dsx = ds * nx;
  const Real dsy = ds * ny;
  const Real dsz = ds * nz;

  // d/d(grad eta)_x
  _dS(0, 0, 0) = (ny * ny + nz * nz) * dsx;                // (ny * ny + nz * nz) * s;
  _dS(0, 1, 0) = _dS(0, 0, 1) = -ny * s - nx * ny * dsx;   // -nx * ny * s;
  _dS(0, 1, 1) = 2.0 * nx * s + (nx * nx + nz * nz) * dsx; // (nx * nx + nz * nz) * s;
  _dS(0, 2, 0) = _dS(0, 0, 2) = -nz * s - nx * nz * dsx;   // -nx * nz * s;
  _dS(0, 2, 1) = _dS(0, 1, 2) = -ny * nz * dsx;            // -ny * nz * s;
  _dS(0, 2, 2) = 2.0 * nx * s + (nx * nx + ny * ny) * dsx; // (nx * nx + ny * ny) * s;

  // d/d(grad eta)_y
  _dS(1, 0, 0) = 2.0 * ny * s + (ny * ny + nz * nz) * dsy; // (ny * ny + nz * nz) * s;
  _dS(1, 1, 0) = _dS(1, 0, 1) = -nx * s - nx * ny * dsy;   // -nx * ny * s;
  _dS(1, 1, 1) = (nx * nx + nz * nz) * dsy;                // (nx * nx + nz * nz) * s;
  _dS(1, 2, 0) = _dS(1, 0, 2) = -nx * nz * dsy;            // -nx * nz * s;
  _dS(1, 2, 1) = _dS(1, 1, 2) = -nz * s - ny * nz * dsy;   // -ny * nz * s;
  _dS(1, 2, 2) = 2.0 * ny * s + (nx * nx + ny * ny) * dsy; // (nx * nx + ny * ny) * s;

  // d/d(grad eta)_z
  _dS(2, 0, 0) = 2.0 * nz * s + (ny * ny + nz * nz) * dsz; // (ny * ny + nz * nz) * s;
  _dS(2, 1, 0) = _dS(2, 0, 1) = -nx * ny * dsz;            // -nx * ny * s;
  _dS(2, 1, 1) = 2.0 * nz * s + (nx * nx + nz * nz) * dsz; // (nx * nx + nz * nz) * s;
  _dS(2, 2, 0) = _dS(2, 0, 2) = -nx * s - nx * nz * dsz;   // -nx * nz * s;
  _dS(2, 2, 1) = _dS(2, 1, 2) = -ny * s - ny * nz * dsz;   // -ny * nz * s;
  _dS(2, 2, 2) = (nx * nx + ny * ny) * dsz;                // (nx * nx + ny * ny) * s;

  return _L[_qp] * 0.5 * _dS.doubleContraction(_strain[_qp]) * _grad_test[_i][_qp];
}

Real
ACInterfaceStress::computeQpJacobian()
{
  // no interface, return zero stress
  const Real grad_norm_sq = _grad_u[_qp].norm_sq();
  if (grad_norm_sq < libMesh::TOLERANCE)
    return 0.0;

  const Real grad_norm = std::sqrt(grad_norm_sq);

  const Real nx = _grad_u[_qp](0);
  const Real ny = _grad_u[_qp](1);
  const Real nz = _grad_u[_qp](2);

  const Real px = _grad_phi[_j][_qp](0);
  const Real py = _grad_phi[_j][_qp](1);
  const Real pz = _grad_phi[_j][_qp](2);

  const Real s = _stress / grad_norm;
  const Real ds = -_stress / (grad_norm * grad_norm_sq);
  const Real dsx = ds * nx;
  const Real dsy = ds * ny;
  const Real dsz = ds * nz;

  const Real dus = ds * (nx * px + ny * py + pz * nz);

  const Real b = -3.0 * nx * px - 3.0 * ny * py - 3.0 * nz * pz;
  const Real dudsx = ds * nx / grad_norm_sq * b + ds * px;
  const Real dudsy = ds * ny / grad_norm_sq * b + ds * py;
  const Real dudsz = ds * nz / grad_norm_sq * b + ds * pz;

  // d/du d/d(grad eta)_x
  _ddS(0, 0, 0) = (2.0 * ny * py + 2.0 * nz * pz) * dsx + (ny * ny + nz * nz) * dudsx;
  _ddS(0, 1, 0) = _ddS(0, 0, 1) =
      -py * s - ny * dus - px * ny * dsx - nx * py * dsx - nx * ny * dudsx;
  _ddS(0, 1, 1) = 2.0 * px * s + 2.0 * nx * dus + (2.0 * nx * px + 2.0 * nz * pz) * dsx +
                  (nx * nx + nz * nz) * dudsx;
  _ddS(0, 2, 0) = _ddS(0, 0, 2) =
      -pz * s - nz * dus - px * nz * dsx - nx * pz * dsx - nx * nz * dudsx;
  _ddS(0, 2, 1) = _ddS(0, 1, 2) = -py * nz * dsx - ny * pz * dsx - ny * nz * dudsx;
  _ddS(0, 2, 2) = 2.0 * px * s + 2.0 * nx * dus + (2.0 * nx * px + 2.0 * ny * py) * dsx +
                  (nx * nx + ny * ny) * dudsx;

  // d/du d/d(grad eta)_y
  _ddS(1, 0, 0) = 2.0 * py * s + 2.0 * ny * dus + (2.0 * ny * py + 2.0 * nz * pz) * dsy +
                  (ny * ny + nz * nz) * dudsy;
  _ddS(1, 1, 0) = _ddS(1, 0, 1) =
      -px * s - nx * dus - px * ny * dsy - nx * py * dsy - nx * ny * dudsy;
  _ddS(1, 1, 1) = (2.0 * nx * px + 2.0 * nz * pz) * dsy + (nx * nx + nz * nz) * dudsy;
  _ddS(1, 2, 0) = _ddS(1, 0, 2) = -px * nz * dsy - nx * pz * dsy - nx * nz * dudsy;
  _ddS(1, 2, 1) = _ddS(1, 1, 2) =
      -pz * s - nz * dus - py * nz * dsy - ny * pz * dsy - ny * nz * dudsy;
  _ddS(1, 2, 2) = 2.0 * py * s + 2.0 * ny * dus + (2.0 * nx * px + 2.0 * ny * py) * dsy +
                  (nx * nx + ny * ny) * dudsy;

  // d/du d/d(grad eta)_z
  _ddS(2, 0, 0) = 2.0 * pz * s + 2.0 * nz * dus + (2.0 * ny * py + 2.0 * nz * pz) * dsz +
                  (ny * ny + nz * nz) * dudsz;
  _ddS(2, 1, 0) = _ddS(2, 0, 1) = -px * ny * dsz - nx * py * dsz - nx * ny * dudsz;
  _ddS(2, 1, 1) = 2.0 * pz * s + 2.0 * nz * dus + (2.0 * nx * px + 2.0 * nz * pz) * dsz +
                  (nx * nx + nz * nz) * dudsz;
  _ddS(2, 2, 0) = _ddS(2, 0, 2) =
      -px * s - nx * dus - px * nz * dsz - nx * pz * dsz - nx * nz * dudsz;
  _ddS(2, 2, 1) = _ddS(2, 1, 2) =
      -py * s - ny * dus - py * nz * dsz - ny * pz * dsz - ny * nz * dudsz;
  _ddS(2, 2, 2) = (2.0 * nx * px + 2.0 * ny * py) * dsz + (nx * nx + ny * ny) * dudsz;

  return _L[_qp] * 0.5 * _ddS.doubleContraction(_strain[_qp]) * _grad_test[_i][_qp];
}
