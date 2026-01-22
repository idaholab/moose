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
 * Free outflow boundary flux for SWE: uses physical flux $F(U) \cdot n$.
 */
class SWEFreeOutflowBoundaryFlux : public BoundaryFluxBase
{
public:
  static InputParameters validParams();

  SWEFreeOutflowBoundaryFlux(const InputParameters & parameters);
  virtual ~SWEFreeOutflowBoundaryFlux();

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
  void fill_dF(const std::vector<Real> & U,
               Real nx,
               Real ny,
               Real g,
               Real h_eps,
               DenseMatrix<Real> & J) const;

  /// gravity and dry depth for consistency if needed later
  const Real _h_eps;
};
