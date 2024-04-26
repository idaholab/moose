//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesPhysicsBase.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"

class WCNSFVFluidHeatTransferPhysics;
class WCNSFVScalarTransportPhysics;

/**
 * Creates all the objects needed to add a turbulence model to an incompressible /
 * weakly-compressible Navier Stokes finite volume flow simulation
 */
class WCNSFVTurbulencePhysics final : public NavierStokesPhysicsBase,
                                      public WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSFVTurbulencePhysics(const InputParameters & parameters);

  /// Whether a turbulence model is in use
  bool hasTurbulenceModel() const { return _turbulence_model != "none"; }

protected:
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

private:
  void addNonlinearVariables() override;
  void addAuxiliaryVariables() override;
  void addFVKernels() override;
  void addAuxiliaryKernels() override;
  void addMaterials() override;

  /**
   * Functions adding kernels for the turbulence equation(s)
   */
  void addFlowTurbulenceKernels();
  void addFluidEnergyTurbulenceKernels();
  void addScalarAdvectionTurbulenceKernels();

  /// Turbulence model to create the equation(s) for
  const MooseEnum _turbulence_model;

  bool _has_flow_equations;
  bool _has_energy_equation;
  bool _has_scalar_equations;

  /// The heat advection physics to add turbulent mixing for
  const WCNSFVFluidHeatTransferPhysics * _fluid_energy_physics;
  /// The scalar advection physics to add turbulent mixing for
  const WCNSFVScalarTransportPhysics * _scalar_transport_physics;

private:
  /// Name of the mixing length auxiliary variable
  const VariableName _mixing_length_name;
  /// List of boundaries to act as walls for turbulence models
  std::vector<BoundaryName> _turbulence_walls;
};
