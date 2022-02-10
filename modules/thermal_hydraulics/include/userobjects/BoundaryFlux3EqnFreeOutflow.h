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
#include "SinglePhaseFluidProperties.h"

/**
 * Computes the outflow boundary flux directly for the 1-D, 1-phase, variable-area Euler equations
 */
class BoundaryFlux3EqnFreeOutflow : public BoundaryFluxBase
{
public:
  BoundaryFlux3EqnFreeOutflow(const InputParameters & parameters);

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<Real> & U1,
                        const RealVectorValue & normal,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<Real> & U1,
                            const RealVectorValue & normal,
                            DenseMatrix<Real> & jac1) const override;

protected:
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
