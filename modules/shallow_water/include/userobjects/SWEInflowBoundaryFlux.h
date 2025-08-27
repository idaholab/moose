//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryFluxBase.h"

/**
 * Inflow boundary flux for SWE: uses Rusanov/HLL flux between internal state
 * and prescribed external state (h_in, un_in along normal).
 */
class SWEInflowBoundaryFlux : public BoundaryFluxBase
{
public:
  static InputParameters validParams();

  SWEInflowBoundaryFlux(const InputParameters & parameters);
  virtual ~SWEInflowBoundaryFlux();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<Real> & uvec1,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<Real> & uvec1,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1) const override;

protected:
  const Real _g;
  const Real _h_eps;
  const Real _h_in;
  const Real _un_in;
};
