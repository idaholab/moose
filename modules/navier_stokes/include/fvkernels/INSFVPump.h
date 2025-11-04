//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"
#include "INSFVMomentumResidualObject.h"

/**
 * Body force that contributes to the Rhie-Chow interpolation
 */
class INSFVPump : public FVElementalKernel, public INSFVMomentumResidualObject
{
public:
  INSFVPump(const InputParameters & params);
  static InputParameters validParams();

  void gatherRCData(const Elem &) override {}
  void gatherRCData(const FaceInfo &) override {}

protected:
  ADReal computeQpResidual() override;

  /// Pump Functor Material
  const Moose::Functor<Real> & _pump_volume_force;
};
