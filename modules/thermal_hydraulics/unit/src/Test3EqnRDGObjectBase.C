//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Test3EqnRDGObjectBase.h"
#include "THMIndicesVACE.h"

std::vector<ADReal>
Test3EqnRDGObjectBase::computeConservativeSolution(const std::vector<ADReal> & W,
                                                   const ADReal & A) const
{
  const ADReal & p = W[0];
  const ADReal & T = W[1];
  const ADReal & vel = W[2];

  const ADReal rho = _fp.rho_from_p_T(p, T);
  const ADReal e = _fp.e_from_p_rho(p, rho);
  const ADReal E = e + 0.5 * vel * vel;

  std::vector<ADReal> U(THMVACE1D::N_FLUX_INPUTS, 0.0);
  U[THMVACE1D::RHOA] = rho * A;
  U[THMVACE1D::RHOUA] = rho * vel * A;
  U[THMVACE1D::RHOEA] = rho * E * A;
  U[THMVACE1D::AREA] = A;

  return U;
}
