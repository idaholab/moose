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
#include "NS.h"

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
  /// The names of the boundaries with turbulence wall functions
  std::vector<BoundaryName> turbulenceWalls() const { return _turbulence_walls; }
  /// The turbulence epsilon wall treatment (same for all turbulence walls currently)
  MooseEnum turbulenceEpsilonWallTreatment() const { return _wall_treatment_eps; }
  /// The turbulence temperature wall treatment (same for all turbulence walls currently)
  MooseEnum turbulenceTemperatureWallTreatment() const { return _wall_treatment_temp; }
  /// The name of the turbulent kinetic energy variable
  MooseFunctorName tkeName() const { return _tke_name; }

protected:
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

private:
  virtual void initializePhysicsAdditional() override;
  virtual void actOnAdditionalTasks() override;
  /// Retrieve the other WCNSFVPhysics at play in the simulation to be able
  /// to add the relevant terms (turbulent diffusion notably)
  void retrieveCoupledPhysics();

  virtual void addSolverVariables() override;
  virtual void addAuxiliaryVariables() override;
  virtual void addFVKernels() override;
  virtual void addFVBCs() override;
  virtual void addInitialConditions() override;
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
  /// Turbulence wall treatment for epsilon (same for all walls currently)
  MooseEnum _wall_treatment_eps;
  /// Turbulence wall treatment for temperature (same for all walls currently)
  MooseEnum _wall_treatment_temp;
  /// Name of the turbulent kinetic energy
  const VariableName _tke_name;
  /// Name of the turbulent kinetic energy dissipation
  const VariableName _tked_name;
  /// Name of the turbulence viscosity auxiliary variable (or property)
  const VariableName _turbulent_viscosity_name = NS::mu_t;
};
