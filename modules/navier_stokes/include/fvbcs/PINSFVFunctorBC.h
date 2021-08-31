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
#include "INSFVFlowBC.h"
#include "CentralDifferenceLimiter.h"

/**
 *  Evaluates boundary mass or momentum fluxes through functor evaluation of the superficial
 * velocities, pressure, density, and porosity
 */
class PINSFVFunctorBC : public FVFluxBC, public INSFVFlowBC
{
public:
  static InputParameters validParams();
  PINSFVFunctorBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// The x component of the superficial velocity
  const FunctorInterface<ADReal> & _sup_vel_x;
  /// The y component of the superficial velocity
  const FunctorInterface<ADReal> * const _sup_vel_y;
  /// The z component of the superficial velocity
  const FunctorInterface<ADReal> * const _sup_vel_z;
  /// The pressure
  const FunctorInterface<ADReal> & _pressure;
  /// The density
  const FunctorInterface<ADReal> & _rho;
  /// The porosity
  const FunctorInterface<ADReal> & _eps;
  /// Denotes the equation we're computing the boundary fluxes for. Options are either "mass" or "momentum"
  const MooseEnum _eqn;
  /// If computing the boundary fluxes for the momentum equation, this denotes the component of the
  /// momentum equation we are computing for
  const unsigned int _index;

  /// A "dummy" limiter required to call face-argument functor overloads
  Moose::FV::CentralDifferenceLimiter<ADReal> _cd_limiter;
};
