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
 * by an order parameter eta using n = grad(eta) / | grad(eta) |.
 *
 * At every quadrature point the local frame is:
 *
 *   n  - unit interface normal   (grad(eta) / | grad(eta) |)
 *   t_1 - tangent in the xy-plane (= z x n, normalized)
 *   t_2 - binormal                (= n x t_1)
 *
 * The normal curvatures are then:
 *
 *   kappa_1 = t_1 S t_1    (curvature along t_1, the in-plane tangent)
 *   kappa_2 = t_2 S t_2    (curvature along t_2, the out-of-plane tangent)
 *
 * where  S is the shape operator (Weingarten map).
 *
 * Note: kappa_1 + kappa_2 = mean curvature kappa = div(n)  (as a consistency check).
 *
 * Requires second-order Lagrange (or higher) elements so that the
 * second derivatives of eta are available.
 */
class InterfaceNormalCurvatures : public Material
{
public:
  static InputParameters validParams();
  InterfaceNormalCurvatures(const InputParameters & params);

protected:
  virtual void computeQpProperties() override;

private:
  // -- order parameter ----------------------------------------
  const VariableValue & _eta;
  const VariableGradient & _grad_eta;
  const VariableSecond & _second_eta;

  // -- user parameters ----------------------------------------
  const Real _grad_threshold; /// threshold for grad(eta) (avoids /0)

  // -- material properties produced ---------------------------
  MaterialProperty<Real> & _kappa1; /// normal curvature along t_1
  MaterialProperty<Real> & _kappa2; /// normal curvature along t_2

  // -- optional diagnostics -----------------------------------
  MaterialProperty<Real> & _kappa_mean;
};
