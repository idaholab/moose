//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADBoundaryFluxBase.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes the outflow boundary flux directly for the 1-D, 1-phase, variable-area Euler equations
 */
class ADBoundaryFlux3EqnFreeOutflow : public ADBoundaryFluxBase
{
public:
  ADBoundaryFlux3EqnFreeOutflow(const InputParameters & parameters);

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<ADReal> & U1,
                        const RealVectorValue & normal,
                        std::vector<ADReal> & flux) const override;

protected:
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
