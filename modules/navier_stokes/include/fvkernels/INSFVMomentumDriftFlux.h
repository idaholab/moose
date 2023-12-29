//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MathFVUtils.h"
#include "INSFVFluxKernel.h"
#include "INSFVMomentumResidualObject.h"

class INSFVMomentumDriftFlux : public INSFVFluxKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumDriftFlux(const InputParameters & params);
  using INSFVFluxKernel::gatherRCData;
  void gatherRCData(const FaceInfo & fi) override final;

protected:
  /**
   * Routine to compute this object's strong residual (e.g. not multiplied by area). This routine
   * should also populate the _ae and _an coefficients
   */
  virtual ADReal computeStrongResidual(const bool populate_a_coeffs);

  virtual ADReal computeSegregatedContribution() override;

  /// The dimension of the simulation
  const unsigned int _dim;

  /// Dispersed Phase Density
  const Moose::Functor<ADReal> & _rho_d;

  /// Dispersed Phase Fraction
  const Moose::Functor<ADReal> & _f_d;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_slip;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v_slip;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w_slip;

  /// The face interpolation method for the density
  const Moose::FV::InterpMethod _density_interp_method;

  /// The a coefficient for the element
  ADReal _ae = 0;

  /// The a coefficient for the neighbor
  ADReal _an = 0;
};
