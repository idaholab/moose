/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GAPCONDUCTANCECONSTRAINT_H
#define GAPCONDUCTANCECONSTRAINT_H

#include "FaceFaceConstraint.h"

class GapConductanceConstraint;

template <>
InputParameters validParams<GapConductanceConstraint>();

/**
 * This Constraint implements thermal contact using a "gap
 * conductance" model in which the flux is represented by an
 * independent "Lagrange multiplier" like variable.  This formulation
 * is not derived from a constrained optimization problem, so it is
 * not a Lagrange multiplier formulation in the classic sense, but it
 * does have the benefit of producing an improved approximation to the
 * flux (better than simply differentiating the finite element
 * solution) and is a systematic approach for accurately computing
 * integrals on mismatched grids. For more information on this
 * formulation, see the following references:
 *
 * M. Gitterle, "A dual mortar formulation for finite deformation
 * frictional contact problems including wear and thermal coupling," PhD
 * thesis, Technische Universit\"{a}t M\"{u}nchen, Nov. 2012,
 * https://mediatum.ub.tum.de/doc/1108639/1108639.pdf.
 *
 * S. H\"{u}eber and B. I. Wohlmuth, "Thermo-mechanical contact
 * problems on non-matching meshes," Computer Methods in Applied
 * Mechanics and Engineering, vol. 198, pp. 1338--1350, Mar. 2009,
 * http://dx.doi.org/10.1016/j.cma.2008.11.022.
 *
 * S.~Falletta and B.~P. Lamichhane, "Mortar finite elements for a
 * heat transfer problem on sliding meshes," Calcolo, vol. 46,
 * pp. 131--148, June 2009, http://dx.doi.org/10.1007/s10092-009-0001-1}.
 *
 * The PDF avaialable from http://tinyurl.com/gmmhbe9 explains the
 * formulation in more detail.  In the documentation below, we use the
 * notation from the PDF above, and refer to the "primal" and "LM"
 * equations, where primal refers to the heat transfer equation
 * including the gap heat flux contribution, and "LM" refers to the
 * equation for computing the flux, i.e. the Lagrange multiplier
 * variable. Likewise, the term "primal variable" refers to the
 * temperature variable.
 */
class GapConductanceConstraint : public FaceFaceConstraint
{
public:
  GapConductanceConstraint(const InputParameters & parameters);
  virtual ~GapConductanceConstraint();

protected:
  /**
   * Computes the residual for the LM equation, lambda = (k/l)*(T^(1) - PT^(2)).
   */
  virtual Real computeQpResidual();

  /**
   * Computes the "lambda * (v^(1) - Pv^(2))" residual term in the
   * primal equation.  The res_type flag controls whether the
   * contribution from the master (1) or slave (2) test function is
   * currently being computed.
   */
  virtual Real computeQpResidualSide(Moose::ConstraintType res_type);

  /**
   * Computes the Jacobian of the LM equation wrt lambda, i.e. both
   * phi(j) and test(i) are from the LM space.  This is simply a
   * (negative) mass matrix contribution, due to the structure of the
   * LM equation.
   */
  virtual Real computeQpJacobian();

  /**
   * Handles Jacobian contributions for *both* the LM equation *and* the primal equation.
   * The jac_type flag controls the type of contribution:
   * Master/Master: LM equation Jacobian wrt to T^(1), phi(j) is primal basis, master side, test(i)
   * is LM basis, master side.
   * Master/Slave: LM equation Jacobian wrt T^(2), phi(j) is primal basis, slave side, test(i) is LM
   * basis, slave side.
   * Slave/Master: Primal equation Jacobian wrt lambda, phi(j) is the LM basis, test(i) is the
   * primal basis, master side.
   * Slave/Slave: Primal equation Jacobian wrt lambda, phi(j) is the LM basis, test(i) is the primal
   * basis, slave side.
   */
  virtual Real computeQpJacobianSide(Moose::ConstraintJacobianType jac_type);

  /// Thermal conductivity of the gap medium (e.g. air).
  Real _k;
};

#endif // GAPCONDUCTANCECONSTRAINT_H
