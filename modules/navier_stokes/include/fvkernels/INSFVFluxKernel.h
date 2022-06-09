//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"
#include "INSFVMomentumResidualObject.h"

/**
 * A flux kernel that momentum residual objects that add non-advection flux terms, or more
 * specifically do not call _rc_uo.getVelocity, should inherit from
 */
class INSFVFluxKernel : public FVFluxKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVFluxKernel(const InputParameters & params);

  using INSFVMomentumResidualObject::gatherRCData;
  void gatherRCData(const Elem &) override final {}

  virtual ~INSFVFluxKernel() = default;

  using FVFluxKernel::computeResidual;
  void computeResidual(const FaceInfo &) override final {}
  using FVFluxKernel::computeJacobian;
  void computeJacobian(const FaceInfo &) override final {}
  using FVFluxKernel::computeResidualAndJacobian;
  void computeResidualAndJacobian(const FaceInfo &) override final {}

protected:
  ADReal computeQpResidual() override final
  {
    mooseError("INSFVFluxKernels must implement gatherRCData and not computeQpResidual");
  }

  /**
   * Process into either the system residual or Jacobian
   */
  void processResidualAndJacobian(const ADReal & residual);
};
