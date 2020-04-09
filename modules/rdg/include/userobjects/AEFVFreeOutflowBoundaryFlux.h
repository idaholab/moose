//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryFluxBase.h"

// Forward Declarations

/**
 * Free outflow BC based boundary flux user object
 * for the advection equation
 * using a cell-centered finite volume method
 */
class AEFVFreeOutflowBoundaryFlux : public BoundaryFluxBase
{
public:
  static InputParameters validParams();

  AEFVFreeOutflowBoundaryFlux(const InputParameters & parameters);
  virtual ~AEFVFreeOutflowBoundaryFlux();

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
  /// advective velocity
  const Real _velocity;
};
