//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeInterfaceStress.h"
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeInterfaceStress);

InputParameters
ComputeInterfaceStress::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Stress in the plane of an interface defined by the gradient of an order parameter");
  params.addCoupledVar("v",
                       "Order parameters that define the interface. The interface is the region "
                       "where the gradient of this order parameter is non-zero.");
  params.addRequiredParam<std::vector<Real>>("stress",
                                             "Interfacial planar stress magnitude (one "
                                             "value to apply to all order parameters or one value "
                                             "per order parameter listed in 'v')");
  params.addRangeCheckedParam<std::vector<Real>>(
      "op_range",
      {1.0},
      "op_range > 0.0",
      "Range over which order parameters change across an "
      "interface. By default order parameters are assumed to "
      "vary from 0 to 1");
  params.addParam<MaterialPropertyName>("planar_stress_name",
                                        "extra_stress",
                                        "Material property name for the interfacial planar stress");
  return params;
}

ComputeInterfaceStress::ComputeInterfaceStress(const InputParameters & parameters)
  : Material(parameters),
    _nvar(coupledComponents("v")),
    _grad_v(_nvar),
    _op_range(getParam<std::vector<Real>>("op_range")),
    _stress(getParam<std::vector<Real>>("stress")),
    _planar_stress(
        declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("planar_stress_name")))
{
  if (_stress.size() == 1)
    _stress.assign(_nvar, _stress[0]);
  if (_stress.size() != _nvar)
    paramError("stress", "Supply either one single stress or one per order parameter");

  if (_op_range.size() == 1)
    _op_range.assign(_nvar, _op_range[0]);
  if (_op_range.size() != _nvar)
    paramError("op_range", "Supply either one single op_range or one per order parameter");

  for (MooseIndex(_grad_v) i = 0; i < _nvar; ++i)
  {
    _grad_v[i] = &coupledGradient("v", i);
    _stress[i] /= _op_range[i];
  }
}

void
ComputeInterfaceStress::computeQpProperties()
{
  auto & S = _planar_stress[_qp];
  S.zero();

  // loop over interface variables
  for (MooseIndex(_grad_v) i = 0; i < _nvar; ++i)
  {
    // compute norm square of the order parameter gradient
    const Real grad_norm_sq = (*_grad_v[i])[_qp].norm_sq();

    // gradient square is zero -> no interface -> no interfacial stress contribution
    if (grad_norm_sq < libMesh::TOLERANCE)
      continue;

    const Real nx = (*_grad_v[i])[_qp](0);
    const Real ny = (*_grad_v[i])[_qp](1);
    const Real nz = (*_grad_v[i])[_qp](2);
    const Real s = _stress[i] / std::sqrt(grad_norm_sq);

    S(0, 0) += (ny * ny + nz * nz) * s;
    S(0, 1) += -nx * ny * s;
    S(1, 1) += (nx * nx + nz * nz) * s;
    S(0, 2) += -nx * nz * s;
    S(1, 2) += -ny * nz * s;
    S(2, 2) += (nx * nx + ny * ny) * s;
  }

  // fill in symmetrically
  S(1, 0) = S(0, 1);
  S(2, 0) = S(0, 2);
  S(2, 1) = S(1, 2);
}
