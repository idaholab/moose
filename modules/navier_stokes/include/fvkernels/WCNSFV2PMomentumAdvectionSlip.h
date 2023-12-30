//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumAdvection.h"
#include "PiecewiseByBlockLambdaFunctor.h"

/*
 * Adds momentum kernel coming from the slip velocity in two-phase mixture model
 */
class WCNSFV2PMomentumAdvectionSlip : public INSFVMomentumAdvection
{
public:
  static InputParameters validParams();
  WCNSFV2PMomentumAdvectionSlip(const InputParameters & params);

  using INSFVMomentumAdvection::computeResidual;
  void computeResidual(const FaceInfo & fi) override;

protected:
  /**
   * Helper method that computes the 'a' coefficients and AD residuals
   */
  void computeResidualsAndADataSlip(const FaceInfo & fi);

  /// Dispersed Phase Density
  const Moose::Functor<ADReal> & _rho_d;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_slip;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v_slip;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w_slip;

  /// Particle diameter in the dispersed phase
  const Moose::Functor<ADReal> & _fd;
};
