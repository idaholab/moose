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
#include "INSFVMomentumResidualObject.h"

/**
 * A flux boundary condition that momentum residual objects that add boundary flux terms should
 * inherit from
 */
class INSFVFluxBC : public FVFluxBC, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVFluxBC(const InputParameters & params);

  using INSFVMomentumResidualObject::gatherRCData;
  void gatherRCData(const Elem &) override final {}

  virtual ~INSFVFluxBC() = default;

  void computeResidual(const FaceInfo &) override final {}
  void computeJacobian(const FaceInfo &) override final {}
  void computeResidualAndJacobian(const FaceInfo &) override final {}

  /**
   * Process into either the system residual or Jacobian
   */
  void processResidualAndJacobian(const ADReal & residual);

protected:
  ADReal computeQpResidual() override final
  {
    mooseError("INSFVFluxBCs must implement gatherRCData and not computeQpResidual");
  }
};
