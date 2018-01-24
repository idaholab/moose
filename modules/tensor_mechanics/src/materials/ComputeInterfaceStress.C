/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeInterfaceStress.h"
#include "RankTwoTensor.h"

template <>
InputParameters
validParams<ComputeInterfaceStress>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Stress in the plane of an interface defined by the gradient of an order parameter");
  params.addCoupledVar("v",
                       "Order parameter that defines the interface. The interface is the region "
                       "where the gradient of this order parameter is non-zero.");
  params.addRequiredParam<Real>("stress", "Planar stress");
  params.addRangeCheckedParam<Real>("op_range",
                                    1.0,
                                    "op_range > 0.0",
                                    "Range over which order parameters change across an "
                                    "interface. By default order parameters are assumed to "
                                    "vary from 0 to 1");
  params.addParam<MaterialPropertyName>(
      "planar_stress_name", "extra_stress", "Material property name for the planar stress");
  return params;
}

ComputeInterfaceStress::ComputeInterfaceStress(const InputParameters & parameters)
  : Material(parameters),
    _grad_v(coupledGradient("v")),
    _stress(getParam<Real>("stress") / getParam<Real>("op_range")),
    _planar_stress(
        declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("planar_stress_name")))
{
}

void
ComputeInterfaceStress::computeQpProperties()
{
  auto & S = _planar_stress[_qp];

  // no interface, return zero stress
  const Real grad_norm_sq = _grad_v[_qp].norm_sq();
  if (grad_norm_sq < libMesh::TOLERANCE)
  {
    S.zero();
    return;
  }

  const Real nx = _grad_v[_qp](0);
  const Real ny = _grad_v[_qp](1);
  const Real nz = _grad_v[_qp](2);
  const Real s = _stress / std::sqrt(grad_norm_sq);

  S(0, 0) = (ny * ny + nz * nz) * s;
  S(1, 0) = S(0, 1) = -nx * ny * s;
  S(1, 1) = (nx * nx + nz * nz) * s;
  S(2, 0) = S(0, 2) = -nx * nz * s;
  S(2, 1) = S(1, 2) = -ny * nz * s;
  S(2, 2) = (nx * nx + ny * ny) * s;
}
