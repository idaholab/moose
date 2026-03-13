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
      "Computes the two normal curvatures κ₁ and κ₂ of a diffuse interface "
      "from an order parameter η via n̂ = ∇η/|∇η| and the shape operator "
      "S = −∇n̂.  t̂₁ lies in the xy-plane (ẑ×n̂); t̂₂ = n̂×t̂₁.");

  params.addRequiredCoupledVar("eta", "Order parameter η that defines the interface");

  params.addParam<Real>("regularization", 1e-8,
      "Floor added to |∇η| before division to prevent singularities in "
      "bulk regions where ∇η ≈ 0");

  params.addParam<std::string>("base_name", "",
      "Optional prefix for all material property names");

  return params;
}

InterfaceNormalCurvatures::InterfaceNormalCurvatures(const InputParameters & params)
  : Material(params),
    _eta(coupledValue("eta")),
    _grad_eta(coupledGradient("eta")),
    _second_eta(coupledSecond("eta")),
    _eps(getParam<Real>("regularization")),

    _kappa1   (declareProperty<Real>            (getParam<std::string>("base_name") + "kappa1")),
    _kappa2   (declareProperty<Real>            (getParam<std::string>("base_name") + "kappa2")),
    _kappa_mean(declareProperty<Real>           (getParam<std::string>("base_name") + "kappa_mean")),
    _normal   (declareProperty<RealVectorValue> (getParam<std::string>("base_name") + "interface_normal")),
    _tangent1 (declareProperty<RealVectorValue> (getParam<std::string>("base_name") + "tangent1")),
    _tangent2 (declareProperty<RealVectorValue> (getParam<std::string>("base_name") + "tangent2"))
{
}

void
InterfaceNormalCurvatures::computeQpProperties()
{
  // ── 1.  Interface normal ─────────────────────────────────────────────────
  const RealVectorValue & g = _grad_eta[_qp];
  const Real              g_mag = g.norm();
  const Real              g_reg = g_mag + _eps;

  const RealVectorValue nhat = g / g_reg;   // n̂  (unit normal)
  _normal[_qp] = nhat;

  // ── 2.  Tangent frame ────────────────────────────────────────────────────
  // t̂₁ = ẑ × n̂  projected into the xy-plane.
  // For a normal that is purely along ẑ the cross product vanishes;
  // in that case we fall back to x̂ so the frame is always well-defined.
  static const RealVectorValue ZHAT(0., 0., 1.);
  static const RealVectorValue XHAT(1., 0., 0.);

  RealVectorValue t1 = ZHAT.cross(nhat);
  const Real t1_mag = t1.norm();
  if (t1_mag < 1e-12)
    t1 = XHAT;   // n̂ ≈ ±ẑ: choose x̂ as in-plane tangent
  else
    t1 /= t1_mag;

  const RealVectorValue t2 = nhat.cross(t1);   // already unit length

  _tangent1[_qp] = t1;
  _tangent2[_qp] = t2;

  // ── 3.  Shape operator  S = −∇n̂ ─────────────────────────────────────────
  //
  // n̂ = ∇η / |∇η|,  so its Jacobian is:
  //
  //   ∂n̂ᵢ/∂xⱼ = ( Hᵢⱼ  −  nᵢ (∇η · ∇∂η/∂xⱼ) / |∇η| ) / |∇η|
  //
  // where H = ∇∇η is the Hessian of η.
  // Written compactly:  ∇n̂ = (H − n̂ ⊗ (H·n̂)) / |∇η|
  // and the shape operator  S = −∇n̂.
  //
  // The normal curvature in direction v̂ is  κ(v̂) = v̂ · S · v̂.

  const RankTwoTensor & H = _second_eta[_qp];  // Hessian ∇∇η  (3×3)

  // H·n̂  (matrix-vector product)
  RealVectorValue Hn;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      Hn(i) += H(i, j) * nhat(j);

  // Normal curvature along a unit tangent v̂:
  //   κ(v̂) = v̂ · S · v̂
  //         = −v̂ · (∇n̂) · v̂
  //         = −[ (v̂·H·v̂) − (v̂·n̂)(n̂·H·n̂) ] / |∇η|
  // but since v̂ ⊥ n̂  →  v̂·n̂ = 0, this simplifies to:
  //   κ(v̂) = −(v̂·H·v̂) / |∇η|                               (*)

  auto vHv = [&](const RealVectorValue & v) -> Real
  {
    Real val = 0.;
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        val += v(i) * H(i, j) * v(j);
    return val;
  };

  _kappa1[_qp] = -vHv(t1) / g_reg;   // κ₁  (in-plane tangent)
  _kappa2[_qp] = -vHv(t2) / g_reg;   // κ₂  (out-of-plane tangent)

  // ── 4.  Mean curvature (diagnostic)  κ = κ₁ + κ₂ = ∇·n̂ ─────────────────
  // Trace of shape operator = H.trace()/|∇η| − (n̂·H·n̂)/|∇η|
  const Real nHn = nhat * Hn;          // n̂·H·n̂
  _kappa_mean[_qp] = -(H.tr() - nHn) / g_reg;
}
