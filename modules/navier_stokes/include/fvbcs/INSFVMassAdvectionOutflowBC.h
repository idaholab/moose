//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"
#include "INSFVFullyDevelopedFlowBC.h"

class INSFVVelocityVariable;

/**
 * A class for finite volume fully developed outflow boundary conditions for the mass equation
 * It advects mass at the outflow, and may replace outlet pressure boundary conditions
 * when selecting a mean-pressure approach.
 */
class INSFVMassAdvectionOutflowBC : public FVFluxBC, public INSFVFullyDevelopedFlowBC
{
public:
  static InputParameters validParams();
  INSFVMassAdvectionOutflowBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// x-velocity
  const Moose::Functor<ADReal> & _u;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w;

  /// the dimension of the simulation
  const unsigned int _dim;
};
