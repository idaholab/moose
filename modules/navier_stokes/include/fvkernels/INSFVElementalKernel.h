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
class INSFVElementalKernel : public FVElementalKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVElementalKernel(const InputParameters & params);

  using INSFVMomentumResidualObject::gatherRCData;
  void gatherRCData(const FaceInfo &) override final {}

  virtual ~INSFVElementalKernel() = default;

  void computeResidual() override;
  void computeJacobian() override;
  using FVElementalKernel::computeOffDiagJacobian;
  void computeOffDiagJacobian() override final {}
  void computeResidualAndJacobian() override;

protected:
  ADReal computeQpResidual() override final
  {
    mooseError("INSFVElementalKernels must implement gatherRCData and not computeQpResidual");
  }

  /// Compute the contribution which goes into the residual of the segregated system. This
  /// needs to accomodate the different linearization approaches needed to get the suitable
  /// system matrix contributions when the Jacobian assembly routine is called.
  virtual ADReal computeSegregatedContribution()
  {
    mooseError(
        this->type(),
        " needs to implement computeSegregatedContribution to be usable with a segregated solver!");
  }

  /**
   * Process into either the system residual or Jacobian
   */
  void addResidualAndJacobian(const ADReal & residual, dof_id_type dof);
};
