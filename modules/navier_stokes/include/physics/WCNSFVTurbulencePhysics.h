//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVTurbulencePhysicsBase.h"
#include "NS.h"

/**
 * Creates all the objects needed to add a turbulence model to an incompressible /
 * weakly-compressible Navier Stokes finite volume flow simulation
 */
class WCNSFVTurbulencePhysics final : public WCNSFVTurbulencePhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVTurbulencePhysics(const InputParameters & parameters);

protected:
  virtual void initializePhysicsAdditional() override;
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

private:
  virtual void addSolverVariables() override;
  virtual void addAuxiliaryVariables() override;
  virtual void addFVKernels() override;
  virtual void addFVBCs() override;
  virtual void addAuxiliaryKernels() override;
  virtual void addMaterials() override;

  /**
   * Functions adding kernels for turbulence in the other equation(s)
   */
  void addFlowTurbulenceKernels();
  void addFluidEnergyTurbulenceKernels();
  void addScalarAdvectionTurbulenceKernels();

  /**
   * Functions adding kernels for the k-epsilon to the k-epsilon equations
   */
  void addKEpsilonTimeDerivatives();
  void addKEpsilonAdvection();
  void addKEpsilonDiffusion();
  void addKEpsilonSink();

  /// Name of the mixing length auxiliary variable
  const VariableName _mixing_length_name;
};
