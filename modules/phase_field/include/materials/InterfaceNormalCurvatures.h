//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Computes the two normal curvatures of a diffuse interface defined
 * by an order parameter η using n̂ = ∇η / |∇η|.
 *
 * At every quadrature point the local frame is:
 *
 *   n̂  — unit interface normal   (= ∇η / |∇η|)
 *   t̂₁ — tangent in the xy-plane (= ẑ × n̂, normalised)
 *   t̂₂ — binormal                (= n̂ × t̂₁)
 *
 * The normal curvatures are then:
 *
 *   κ₁ = t̂₁ · S · t̂₁    (curvature along t̂₁, the in-plane tangent)
 *   κ₂ = t̂₂ · S · t̂₂    (curvature along t̂₂, the out-of-plane tangent)
 *
 * where  S = −∇n̂  is the shape operator (Weingarten map).
 *
 * Note: κ₁ + κ₂ = mean curvature κ = ∇·n̂  (as a consistency check).
 *
 * Requires second-order Lagrange (or higher) elements so that the
 * second derivatives of η are available.
 */
class InterfaceNormalCurvatures : public Material
{
public:
  static InputParameters validParams();
  InterfaceNormalCurvatures(const InputParameters & params);

protected:
  virtual void computeQpProperties() override;

private:
  // ── order parameter ────────────────────────────────────────
  const VariableValue  & _eta;        ///< η
  const VariableGradient & _grad_eta; ///< ∇η
  const VariableSecond & _second_eta; ///< ∇∇η  (Hessian)

  // ── user parameters ────────────────────────────────────────
  const Real _eps;   ///< regularisation floor for |∇η| (avoids /0)

  // ── material properties produced ───────────────────────────
  MaterialProperty<Real> & _kappa1; ///< normal curvature along t̂₁
  MaterialProperty<Real> & _kappa2; ///< normal curvature along t̂₂

  // ── optional diagnostics ───────────────────────────────────
  MaterialProperty<Real>         & _kappa_mean;  ///< κ₁+κ₂  (= ∇·n̂)
  MaterialProperty<RealVectorValue> & _normal;   ///< n̂
  MaterialProperty<RealVectorValue> & _tangent1; ///< t̂₁
  MaterialProperty<RealVectorValue> & _tangent2; ///< t̂₂
};
