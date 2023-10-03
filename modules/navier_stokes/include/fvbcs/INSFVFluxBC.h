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

  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;
  void computeResidualAndJacobian(const FaceInfo & fi) override;

  /**
   * Process into either the system residual or Jacobian
   */
  void addResidualAndJacobian(const ADReal & residual);

protected:
  ADReal computeQpResidual() override final;

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
