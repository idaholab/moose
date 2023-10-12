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
  void computeResidual(const FaceInfo & fi) override final;
  using FVFluxKernel::computeJacobian;
  void computeJacobian(const FaceInfo & fi) override final;
  using FVFluxKernel::computeResidualAndJacobian;
  void computeResidualAndJacobian(const FaceInfo & fi) override final;

protected:
  ADReal computeQpResidual() override final;

  /**
   * Process into the system residual
   */
  void addResidual(const Real residual);

  /**
   * Process into either the system residual or Jacobian
   */
  void addResidualAndJacobian(const ADReal & residual);

  /// Compute the contribution which goes into the residual of the segregated system. This
  /// needs to accomodate the different linearization approaches needed to get the suitable
  /// system matrix contributions when the Jacobian assembly routine is called.
  virtual ADReal computeSegregatedContribution()
  {
    mooseError("computeSegregatedContribution not implemented for ",
               this->type(),
               ". This function needs to be implemented to be able to use this object with a "
               "segregated solver!");
  }
};
