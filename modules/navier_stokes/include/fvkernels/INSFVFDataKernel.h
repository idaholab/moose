//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * An elemental kernel that momentum residual objects that add body forces should inherit from
 */
class INSFVFDataKernel : public FVElementalKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVFDataKernel(const InputParameters & params);

  using INSFVMomentumResidualObject::gatherRCData;
  void gatherRCData(const FaceInfo &) override final {}

  virtual ~INSFVFDataKernel() = default;

  void computeResidual() override final {}
  void computeJacobian() override final {}
  void computeOffDiagJacobian() override final {}

protected:
  ADReal computeQpResidual() override final
  {
    mooseError("INSFVFDataKernels must implement gatherRCData and not computeQpResidual");
  }

private:
  using FVElementalKernel::_current_elem;
};
