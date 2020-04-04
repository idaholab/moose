//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSurfaceTensionKKS.h"
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeSurfaceTensionKKS);

InputParameters
ComputeSurfaceTensionKKS::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Surface tension of an interface defined by the gradient of an order parameter");
  params.addCoupledVar("v",
                       "Order parameter that defines the interface, assumed to vary from 0 to 1.");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "Gradient energy coefficient");
  params.addParam<MaterialPropertyName>("g", "g", "Barrier Function Material that provides g(eta)");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  params.addParam<MaterialPropertyName>("planar_stress_name",
                                        "extra_stress",
                                        "Material property name for the interfacial planar stress");
  return params;
}

ComputeSurfaceTensionKKS::ComputeSurfaceTensionKKS(const InputParameters & parameters)
  : Material(parameters),
    _v(coupledValue("v")),
    _grad_v(coupledGradient("v")),
    _kappa(getMaterialProperty<Real>("kappa_name")),
    _g(getMaterialProperty<Real>("g")),
    _w(getParam<Real>("w")),
    _planar_stress(
        declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("planar_stress_name")))
{
}

void
ComputeSurfaceTensionKKS::computeQpProperties()
{
  auto & S = _planar_stress[_qp];
  S.zero();

  // compute norm square of the order parameter gradient
  const Real grad_norm_sq = _grad_v[_qp].norm_sq();

  const Real nx = _grad_v[_qp](0);
  const Real ny = _grad_v[_qp](1);
  const Real nz = _grad_v[_qp](2);
  Real fsum = _w * _g[_qp] + 0.5 * _kappa[_qp] * grad_norm_sq;

  S(0, 0) += fsum - _kappa[_qp] * nx * nx;
  S(0, 1) += -_kappa[_qp] * nx * ny;
  S(1, 1) += fsum - _kappa[_qp] * ny * ny;
  S(0, 2) += -_kappa[_qp] * nx * nz;
  S(1, 2) += -_kappa[_qp] * ny * nz;
  S(2, 2) += fsum - _kappa[_qp] * nz * nz;

  // fill in symmetrically
  S(1, 0) = S(0, 1);
  S(2, 0) = S(0, 2);
  S(2, 1) = S(1, 2);
}
