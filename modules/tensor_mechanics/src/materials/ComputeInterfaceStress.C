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
  params.addParam<Real>("op_range",
                        1.0,
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
  const Real _grad_norm_sq = _grad_v[_qp].norm_sq();
  _planar_stress[_qp].zero();

  // no interface, return zero stress
  if (_grad_norm_sq < libMesh::TOLERANCE)
    return;

  const Real _grad_norm = std::sqrt(_grad_norm_sq);
  RealGradient _col[3];
  _col[0] = _grad_v[_qp] / _grad_norm;

  // get two linearly independent vectors
  unsigned int max_component = 0;
  for (unsigned int i = 1; i < 3; ++i)
    if (std::abs(_col[0](i)) > std::abs(_col[0](max_component)))
      max_component = i;

  _col[1]((max_component + 1) % 3) = 1.0;
  _col[2]((max_component + 2) % 3) = 1.0;

  // make the two vectors perpendicular to _grad_v (modified Gram-Schmidt)
  _col[1] -= (_col[1] * _col[0]) / _col[0].norm_sq() * _col[0];
  _col[2] -= (_col[2] * _col[0]) / _col[0].norm_sq() * _col[0];
  _col[2] -= (_col[2] * _col[1]) / _col[1].norm_sq() * _col[1];

  // normalize new basis vectors
  _col[1] /= _col[1].norm();
  _col[2] /= _col[2].norm();

  // build Eigenvalue matrix M (only set in-plane stress)
  _planar_stress[_qp](1, 1) = _stress;
  _planar_stress[_qp](2, 2) = _stress;

  // build S matrix for coordinate transformation
  RankTwoTensor S = RankTwoTensor::initializeFromColumns(_col[0], _col[1], _col[2]);

  // basis change into cartesian coordinates (and scale with gradient norm)
  _planar_stress[_qp] = (S * _planar_stress[_qp] * S.transpose()) * _grad_norm;
}
