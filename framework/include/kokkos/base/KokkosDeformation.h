//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVariableValue.h"

class Coupleable;

namespace Moose::Kokkos
{

/**
 * Kinematics of the motion x = X + u(X) at a quadrature point.
 *
 * Kokkos objects integrate on the reference configuration. An object whose weak form is posed on
 * the current configuration pulls its integrand back with
 *
 *   dv = J dV,  grad_x = F^{-T} grad_X,  n da = J F^{-T} N dA,
 *
 * where F = I + grad_X u is the deformation gradient. For an isoparametric displacement field this
 * reproduces the quadrature of the same integrand on a mesh whose nodes are moved by u.
 */
struct Kinematics
{
  /**
   * Inverse of the deformation gradient
   */
  Real33 F_inv;
  /**
   * Ratio of the current to the reference volume measure, including the change of the
   * coordinate transform factor (e.g. 2 pi r in axisymmetric coordinates) between X and x
   */
  Real J;
  /**
   * Gradient of the logarithm of the coordinate transform factor at the current position
   */
  Real3 grad_log_coord;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Pull a gradient with respect to the reference position forward to the current position
   * @param grad_X The gradient with respect to the reference position
   * @returns The gradient with respect to the current position, F^{-T} grad_X
   */
  KOKKOS_FUNCTION Real3 grad(const Real3 grad_X) const { return F_inv.transpose() * grad_X; }
  /**
   * Get the ratio of the current to the reference area measure of a side
   * @param N The reference unit normal of the side
   * @returns The area ratio, J |F^{-T} N|
   */
  KOKKOS_FUNCTION Real areaRatio(const Real3 N) const { return J * grad(N).norm(); }
  /**
   * Get the derivative of J with respect to a displacement degree of freedom
   * @param c The displacement component of the degree of freedom
   * @param phi The shape function of the degree of freedom
   * @param grad_X_phi The reference gradient of the shape function
   * @returns The derivative, J ((grad_x phi)_c + phi (grad_x log rho)_c)
   */
  KOKKOS_FUNCTION Real dJ(const unsigned int c, const Real phi, const Real3 grad_X_phi) const
  {
    return J * (grad(grad_X_phi)(c) + phi * grad_log_coord(c));
  }
#endif
};

/**
 * Evaluates the kinematics of the current configuration from coupled displacement variables.
 * Each coupled component c displaces the reference position along coordinate direction c.
 */
class Deformation
{
public:
  /**
   * Default constructor, describing the identity motion
   */
  Deformation() = default;
  /**
   * Constructor
   * @param object The object coupling the displacements
   * @param param The coupled variable parameter holding the displacements
   */
  Deformation(const Coupleable & object, const std::string & param = "displacements");

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get whether the displacements were coupled
   * @returns Whether the displacements were coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _disp; }
  /**
   * Evaluate the kinematics at a quadrature point
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   * @returns The kinematics
   */
  KOKKOS_FUNCTION Kinematics operator()(Datum & datum, const unsigned int qp) const;
  /**
   * Get the displacement component discretized by the Jacobian column variable
   * @param datum The AssemblyDatum object of the current thread
   * @returns The displacement component, or libMesh::invalid_uint if the column variable is not a
   * displacement
   */
  KOKKOS_FUNCTION unsigned int component(const AssemblyDatum & datum) const;
#endif

private:
  /**
   * Displacement values
   */
  VariableValue _disp;
  /**
   * Displacement gradients with respect to the reference position
   */
  VariableGradient _grad_disp;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline Kinematics
Deformation::operator()(Datum & datum, const unsigned int qp) const
{
  const auto X = datum.q_point(qp);

  Real33 F;
  auto x = X;

  for (unsigned int d = 0; d < 3; ++d)
    F(d, d) = 1;

  if (_disp)
    for (unsigned int c = 0; c < _disp.variable().components(); ++c)
    {
      const auto grad_u = _grad_disp(datum, qp, c);

      for (unsigned int d = 0; d < 3; ++d)
        F(c, d) += grad_u(d);

      x(c) += _disp(datum, qp, c);
    }

  const auto & assembly = datum.assembly();
  const auto subdomain = datum.subdomain();
  const auto coord = assembly.coordTransformFactor(subdomain, x);

  Kinematics kinematics;
  kinematics.F_inv = F.inverse();
  kinematics.J = F.determinant() * coord / assembly.coordTransformFactor(subdomain, X);
  kinematics.grad_log_coord = (1 / coord) * assembly.coordTransformFactorGradient(subdomain, x);

  return kinematics;
}

KOKKOS_FUNCTION inline unsigned int
Deformation::component(const AssemblyDatum & datum) const
{
  if (_disp)
  {
    const auto & var = _disp.variable();

    for (unsigned int c = 0; c < var.components(); ++c)
      if (var.sys(c) == datum.sys() && var.var(c) == datum.jvar())
        return c;
  }

  return libMesh::invalid_uint;
}
#endif

} // namespace Moose::Kokkos
