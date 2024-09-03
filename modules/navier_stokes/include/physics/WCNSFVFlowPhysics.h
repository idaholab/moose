//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVFlowPhysicsBase.h"
#include "WCNSFVTurbulencePhysics.h"

/**
 * Creates all the objects needed to solve the Navier Stokes mass and momentum equations
 */
class WCNSFVFlowPhysics final : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVFlowPhysics(const InputParameters & parameters);

  /// Get the name of the linear friction coefficient. Returns an empty string if no friction.
  virtual MooseFunctorName getLinearFrictionCoefName() const override;
  /// Return the name of the Rhie Chow user object
  UserObjectName rhieChowUOName() const override;
  /// Return the number of algebraic ghosting layers needed
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

protected:
  void initializePhysicsAdditional() override;
  void actOnAdditionalTasks() override;

  // Used by derived THMWCNSFVFlowPhysics
  // TODO: make sure list is minimal
  virtual void addSolverVariables() override;
  void addInitialConditions() override;
  void addMaterials() override;
  void addUserObjects() override;

  /*
   * Add an inlet
   * @param boundary_name inlet boundary to add
   * @param inlet_type type of the inlet boundary condition
   * @param inlet_functor functor providing the flux values for the boundary conditions
   */
  void addInletBoundary(const BoundaryName & boundary_name,
                        const MooseEnum & inlet_type,
                        const MooseFunctorName & inlet_functor);

  /*
   * Add an outlet
   * @param boundary_name outlet boundary to add
   * @param outlet_type type of the outlet boundary condition
   * @param outlet_functor functor providing the flux values for the boundary conditions
   */
  void addOutletBoundary(const BoundaryName & boundary_name,
                         const MooseEnum & outlet_type,
                         const MooseFunctorName & outlet_functor);

private:
  void addFVKernels() override;
  void addFVBCs() override;
  void addCorrectors() override;
  void addPostprocessors() override;

  /// Function adding kernels for the incompressible continuity equation
  void addINSMassKernels();
  /// Function adding the pressure constraint
  void addINSPressurePinKernel();

  /**
   * Functions adding kernels for the incompressible momentum equation
   * If the material properties are not constant, these can be used for
   * weakly-compressible simulations (except the Boussinesq kernel) as well.
   */
  void addINSMomentumTimeKernels();
  void addINSMomentumViscousDissipationKernels();
  void addINSMomentumMixingLengthKernels();
  void addINSMomentumAdvectionKernels();
  void addINSMomentumPressureKernels() override;
  void addINSMomentumGravityKernels() override;
  void addINSMomentumBoussinesqKernels() override;
  void addINSMomentumFrictionKernels();

  /// Functions which add time kernels for transient, weakly-compressible simulations.
  void addWCNSMassTimeKernels();
  void addWCNSMomentumTimeKernels();

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addINSInletBC() override;
  void addINSOutletBC() override;
  void addINSWallsBC() override;

  /// Return whether a Forchheimer friction model is in use
  bool hasForchheimerFriction() const override;

  /// Function which adds the RhieChow interpolator user objects for weakly and incompressible formulations
  void addRhieChowUserObjects() override;
  /// Checks that sufficient Rhie Chow coefficients have been defined for the given dimension, used
  /// for scalar or temperature advection by auxiliary variables
  void checkRhieChowFunctorsDefined() const;

  /// The number of smoothing layers if that treatment is used on porosity
  const unsigned _porosity_smoothing_layers;

<<<<<<< HEAD
=======
  /// Velocity names
  const std::vector<std::string> _velocity_names;
  /// Pressure name
  const NonlinearVariableName _pressure_name;
  /// Fluid temperature name
  const NonlinearVariableName _fluid_temperature_name;

  /// Name of the density material property
  const MooseFunctorName _density_name;
  /// Name of the density material property used for gravity and Boussinesq terms
  const MooseFunctorName _density_gravity_name;
  /// Name of the dynamic viscosity material property
  const MooseFunctorName _dynamic_viscosity_name;

  /// The velocity face interpolation method for advecting other quantities
  const MooseEnum _velocity_interpolation;
  /// The momentum face interpolation method for being advected
  const MooseEnum _momentum_face_interpolation;

  /// Can be set to a coupled turbulence physics
  const WCNSFVTurbulencePhysics * _turbulence_physics;

  /// Boundaries with a flow inlet specified on them
  std::vector<BoundaryName> _inlet_boundaries;
  /// Boundaries with a flow outlet specified on them
  std::vector<BoundaryName> _outlet_boundaries;
  /// Boundaries which define a wall (slip/noslip/etc.)
  const std::vector<BoundaryName> _wall_boundaries;

  /// Momentum inlet boundary types
  std::map<BoundaryName, MooseEnum> _momentum_inlet_types;
  /// Momentum outlet boundary types
  std::map<BoundaryName, MooseEnum> _momentum_outlet_types;
  /// Momentum wall boundary types
  MultiMooseEnum _momentum_wall_types;

  /// Postprocessors describing the momentum inlet for each boundary. Indexing based on the number of flux boundaries
  std::vector<PostprocessorName> _flux_inlet_pps;
  /// Direction of each flux inlet. Indexing based on the number of flux boundaries
  std::vector<Point> _flux_inlet_directions;

  /// Functors describing the momentum inlet for each boundary. See matching index momentum_inlet_types
  /// for the function actually computes
  std::map<BoundaryName, std::vector<MooseFunctorName>> _momentum_inlet_functors;
  /// Functors describing the outlet pressure on each boundary
  std::map<BoundaryName, MooseFunctorName> _pressure_functors;

>>>>>>> 8585ad72a0 (Add an option for adding inlet & outlet BCs to wcnsfv physics from the components)
  /// Subdomains where we want to have volumetric friction
  std::vector<std::vector<SubdomainName>> _friction_blocks;
  /// The friction correlation types used for each block
  std::vector<std::vector<std::string>> _friction_types;
  /// The coefficients used for each item if friction type
  std::vector<std::vector<std::string>> _friction_coeffs;
};
