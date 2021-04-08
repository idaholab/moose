//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

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
class GapConductanceConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  GapConductanceConstraint(const InputParameters & parameters);

protected:
  /**
   * Computes the residual for the LM equation, lambda = (k/l)*(T^(1) - PT^(2)).
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) override;

  /// Thermal conductivity of the gap medium (e.g. air).
  const Real _k;

  /// Minimum gap distance allowed
  const Real _min_gap;

  ///@{ Displacement variables
  const std::vector<std::string> _disp_name;
  const unsigned int _n_disp;
  std::vector<const ADVariableValue *> _disp_secondary;
  std::vector<const ADVariableValue *> _disp_primary;
  ///@}
};
