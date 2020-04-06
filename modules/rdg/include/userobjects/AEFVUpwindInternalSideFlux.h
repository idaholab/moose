//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideFluxBase.h"

// Forward Declarations

/**
 * Upwind numerical flux scheme
 * for the advection equation
 * using a cell-centered finite volume method
 */
class AEFVUpwindInternalSideFlux : public InternalSideFluxBase
{
public:
  static InputParameters validParams();

  AEFVUpwindInternalSideFlux(const InputParameters & parameters);
  virtual ~AEFVUpwindInternalSideFlux();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        dof_id_type ineig,
                        const std::vector<Real> & uvec1,
                        const std::vector<Real> & uvec2,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            dof_id_type ineig,
                            const std::vector<Real> & uvec1,
                            const std::vector<Real> & uvec2,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const override;

protected:
  /// advective velocity
  const Real _velocity;
};
