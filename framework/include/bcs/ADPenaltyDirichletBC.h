//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Weakly enforce a Dirichlet BC using a penalty term. This class is
 * an alternative to the DirichletBC that maintains the symmetry (if
 * any) present in the original problem, and does not involve
 * explicitly zeroing matrix rows for its implementation. The main
 * drawback of this approach is that the penalty parameter must tend
 * to infinity in order for the constraint to be satisfied in the
 * limit as h->0, and this causes the Jacobian matrix to be
 * ill-conditioned.
 *
 * The weak form contribution for this term is:
 *
 * \f$ (p (u - g), \psi)_{\Gamma} \f$,
 *
 * where:
 * p = penalty parameter (user-selectable)
 * u = the unknown
 * g = Dirichlet data (given)
 * Gamma = the part of the boundary where the penalty BC is applied.
 */
class ADPenaltyDirichletBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  ADPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Penalty value
  const Real _p;

  /// Value of u on the boundary
  const Real & _v;
};
