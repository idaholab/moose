//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceNormalCurvatures.h"
#include "Assembly.h"

registerMooseObject("PhaseFieldApp", InterfaceNormalCurvatures);

InputParameters
InterfaceNormalCurvatures::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Computes the two normal curvatures kappa_1 and kappa_2 of a diffuse interface "
      "from an order parameter eta.");
  params.addRequiredCoupledVar("eta", "Order parameter that defines the interface");
  params.addParam<Real>("regularization",
                        1e-8,
                        "Floor added to |grad(eta)| before division to prevent singularities in "
                        "bulk regions");
  params.addParam<std::string>("base_name", "", "Optional prefix for all material property names");
  return params;
}

InterfaceNormalCurvatures::InterfaceNormalCurvatures(const InputParameters & params)
  : Material(params),
    _eta(coupledValue("eta")),
    _grad_eta(coupledGradient("eta")),
    _second_eta(coupledSecond("eta")),
    _eps(getParam<Real>("regularization")),
    _kappa1(declareProperty<Real>(getParam<std::string>("base_name") + "kappa1")),
    _kappa2(declareProperty<Real>(getParam<std::string>("base_name") + "kappa2")),
    _kappa_mean(declareProperty<Real>(getParam<std::string>("base_name") + "kappa_mean"))
{
}

void
InterfaceNormalCurvatures::computeQpProperties()
{
  // -- 1.  Interface normal -------------------------------------------------
  const RealVectorValue & g = _grad_eta[_qp];
  const Real g_mag = g.norm();
  const Real g_reg = g_mag + _eps;

  const RealVectorValue nhat = g / g_reg; // n  (unit normal)

  // -- 2.  Tangent frame ----------------------------------------------------
  static const RealVectorValue ZHAT(0., 0., 1.);
  static const RealVectorValue XHAT(1., 0., 0.);

  RealVectorValue t1 = ZHAT.cross(nhat);
  const Real t1_mag = t1.norm();
  if (t1_mag < 1e-12)
    t1 = XHAT; // n = +/- z: choose x as in-plane tangent
  else
    t1 /= t1_mag;

  const RealVectorValue t2 = nhat.cross(t1); // already unit length

  // -- 3.  Shape operator  S ------------------------------------------------

  const RankTwoTensor & H = _second_eta[_qp]; // Hessian (3 x 3)

  RealVectorValue Hn;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      Hn(i) += H(i, j) * nhat(j);

  // Normal curvature along a unit tangent v:
  auto vHv = [&](const RealVectorValue & v) -> Real
  {
    Real val = 0.;
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        val += v(i) * H(i, j) * v(j);
    return val;
  };

  _kappa1[_qp] = -vHv(t1) / g_reg; // in-plane tangent
  _kappa2[_qp] = -vHv(t2) / g_reg; // out-of-plane tangent

  // -- 4.  Mean curvature ---------------------------------------------------

  const Real nHn = nhat * Hn;
  _kappa_mean[_qp] = -(H.tr() - nHn) / g_reg;
}
