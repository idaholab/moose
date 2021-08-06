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

class INSFVMomentumDiffusion : public INSFVFluxKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumDiffusion(const InputParameters & params);
  using INSFVFluxKernel::gatherRCData;
  void gatherRCData(const FaceInfo & fi) override final;

protected:
  /**
   * Routine to compute this object's strong residual (e.g. not multipled by area). This routine
   * should also populate the _ae and _an coefficients
   */
  virtual ADReal computeStrongResidual();

  /// The dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// The a coefficient for the element
  ADReal _ae = 0;

  /// The a coefficient for the neighbor
  ADReal _an = 0;
};
