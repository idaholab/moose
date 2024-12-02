//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVFluidHeatTransferPhysicsBase.h"
#include "NS.h"

/**
 * Creates all the objects needed to solve the Navier Stokes energy equation
 */
class WCNSFVFluidHeatTransferPhysics final : public WCNSFVFluidHeatTransferPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVFluidHeatTransferPhysics(const InputParameters & parameters);

protected:
private:
  virtual void addSolverVariables() override;

  /**
   * Functions adding kernels for the incompressible / weakly compressible energy equation
   * If the material properties are not constant, some of these can be used for
   * weakly-compressible simulations as well.
   */
  void addINSEnergyTimeKernels() override;
  void addWCNSEnergyTimeKernels() override;
  void addINSEnergyHeatConductionKernels() override;
  void addINSEnergyAdvectionKernels() override;
  void addINSEnergyAmbientConvection() override;
  void addINSEnergyExternalHeatSource() override;

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addINSEnergyInletBC() override;
  void addINSEnergyWallBC() override;
  void addINSEnergyOutletBC() override {}
};
