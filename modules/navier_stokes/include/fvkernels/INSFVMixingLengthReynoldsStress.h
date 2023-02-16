//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVFluxKernel.h"
#include "INSFVMomentumResidualObject.h"

// Forward declare variable class
class INSFVVelocityVariable;

class INSFVMixingLengthReynoldsStress : public INSFVFluxKernel
{
public:
  static InputParameters validParams();

  INSFVMixingLengthReynoldsStress(const InputParameters & params);

  using INSFVFluxKernel::gatherRCData;
  void gatherRCData(const FaceInfo &) override final;

protected:
  /**
   * Routine to compute this object's strong residual (e.g. not multipled by area). This routine
   * should also populate the _ae and _an coefficients
   */
  ADReal computeStrongResidual();

  /// The dimension of the simulation
  const unsigned int _dim;

  /// index x|y|z
  const unsigned int _axis_index;

  /// x-velocity
  const Moose::Functor<ADReal> & _u;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Turbulent eddy mixing length
  const Moose::Functor<ADReal> & _mixing_len;

  /// Rhie-Chow element coefficient
  ADReal _ae = 0;

  /// Rhie-Chow neighbor coefficient
  ADReal _an = 0;
};
