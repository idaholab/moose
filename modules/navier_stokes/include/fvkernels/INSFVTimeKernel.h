//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFunctorTimeKernel.h"
#include "INSFVMomentumResidualObject.h"

/**
 * All navier-stokes momentum time derivative terms should inherit from this class
 */
class INSFVTimeKernel : public FVFunctorTimeKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVTimeKernel(const InputParameters & params);

  using INSFVMomentumResidualObject::gatherRCData;
  void gatherRCData(const FaceInfo &) override final {}

  virtual ~INSFVTimeKernel() = default;

  void computeResidual() override final {}
  void computeJacobian() override final {}
  using FVFunctorTimeKernel::computeOffDiagJacobian;
  void computeOffDiagJacobian() override final {}
  void computeResidualAndJacobian() override final {}

protected:
  ADReal computeQpResidual() override final
  {
    mooseError("INSFVTimeKernels must implement gatherRCData and not computeQpResidual");
  }

  /**
   * Process into either the system residual or Jacobian
   */
  void processResidualAndJacobian(const ADReal & residual, dof_id_type dof);

private:
  using FVFunctorTimeKernel::_current_elem;
};
