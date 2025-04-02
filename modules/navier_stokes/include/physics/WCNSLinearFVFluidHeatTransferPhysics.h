//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Creates all the objects needed to solve the Navier Stokes energy equation using a linear finite
 * volume discretization
 */
class WCNSLinearFVFluidHeatTransferPhysics final : public WCNSFVFluidHeatTransferPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVFluidHeatTransferPhysics(const InputParameters & parameters);

private:
  virtual void addSolverVariables() override;
  virtual void addAuxiliaryVariables() override;
  virtual void addAuxiliaryKernels() override;
  virtual void addMaterials() override;

  /**
   * Functions adding kernels for the incompressible / weakly compressible energy equation
   */
  void addEnergyTimeKernels() override;
  void addEnergyHeatConductionKernels() override;
  void addEnergyAdvectionKernels() override;
  void addEnergyAmbientConvection() override;
  void addEnergyExternalHeatSource() override;

  /// Functions adding boundary conditions for the incompressible / weakly compressible energy equation
  void addEnergyInletBC() override;
  void addEnergyWallBC() override;
  void addEnergyOutletBC() override;
  void addEnergySeparatorBC() override {}
};
