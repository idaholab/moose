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
#include "WCNSFVTurbulencePhysics.h"

/**
 * Creates all the objects needed to solve the Navier Stokes mass and momentum equations
 */
class WCNSFVFlowPhysics final : public NavierStokesPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVFlowPhysics(const InputParameters & parameters);

  /// Whether the physics is actually creating the flow equations
  bool hasFlowEquations() const { return _has_flow_equations; }

  /// To interface with other Physics
  const std::vector<std::string> & getVelocityNames() const { return _velocity_names; }
  const NonlinearVariableName & getPressureName() const { return _pressure_name; }
  const NonlinearVariableName & getFluidTemperatureName() const { return _fluid_temperature_name; }
  MooseFunctorName getPorosityFunctorName(bool smoothed) const;

  // Getters to interact with other WCNSFVPhysics classes
  /// Return the compressibility of the flow equations selected
  const MooseEnum & compressibility() const { return _compressibility; }
  /// Return whether a porous medium treatment is applied
  bool porousMediumTreatment() const { return _porous_medium_treatment; }
  /// Return the gravity vector
  RealVectorValue gravityVector() const { return getParam<RealVectorValue>("gravity"); }
  /// Return the name of the density functor
  const MooseFunctorName & densityName() const { return _density_name; }
  /// Return the name of the dynamic viscosity functor
  const MooseFunctorName & dynamicViscosityName() const { return _dynamic_viscosity_name; }
  /// Get the face interpolation method for velocity
  const MooseEnum & getVelocityFaceInterpolationMethod() const { return _velocity_interpolation; }
  /// Get the face interpolation method for velocity
  const MooseEnum & getMomentumFaceInterpolationMethod() const
  {
    return _momentum_face_interpolation;
  }
  /// Get the inlet boundaries
  const std::vector<BoundaryName> & getInletBoundaries() const { return _inlet_boundaries; }
  /// Get the outlet boundaries
  const std::vector<BoundaryName> & getOutletBoundaries() const { return _outlet_boundaries; }
  /// Get the wall boundaries
  const std::vector<BoundaryName> & getWallBoundaries() const { return _wall_boundaries; }
  /// Get the inlet direction if using a flux inlet
  const std::vector<Point> & getFluxInletDirections() const { return _flux_inlet_directions; }
  /// Get the inlet flux postprocessor if using a flux inlet
  const std::vector<PostprocessorName> & getFluxInletPPs() const { return _flux_inlet_pps; }
  /// Get the name of the linear friction coefficient. Returns an empty string if no friction.
  MooseFunctorName getLinearFrictionCoefName() const;
  /// Return the name of the Rhie Chow user object
  UserObjectName rhieChowUOName() const;
  /// Return the number of algebraic ghosting layers needed
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

protected:
  void initializePhysicsAdditional() override;
  void actOnAdditionalTasks() override;

private:
  void addNonlinearVariables() override;
  void addInitialConditions() override;
  void addFVKernels() override;
  void addFVBCs() override;
  void addMaterials() override;
  void addUserObjects() override;
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
  void addINSMomentumPressureKernels();
  void addINSMomentumGravityKernels();
  void addINSMomentumBoussinesqKernels();
  void addINSMomentumFrictionKernels();

  /// Functions which add time kernels for transient, weakly-compressible simulations.
  void addWCNSMassTimeKernels();
  void addWCNSMomentumTimeKernels();

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addINSInletBC();
  void addINSOutletBC();
  void addINSWallsBC();

  /// Return whether a Forchheimer friction model is in use
  bool hasForchheimerFriction() const;

  /// Add material to define the local speed in porous medium flows
  void addPorousMediumSpeedMaterial();
  /// Add material to define the local speed with no porous medium treatment
  void addNonPorousMediumSpeedMaterial();

  /// Function which adds the RhieChow interpolator user objects for weakly and incompressible formulations
  void addRhieChowUserObjects();
  /// Checks that sufficient Rhie Chow coefficients have been defined for the given dimension, used
  /// for scalar or temperature advection by auxiliary variables
  void checkRhieChowFunctorsDefined() const;

  /// Convenience routine to be able to retrieve the actual variable names from their default names
  VariableName getFlowVariableName(const std::string & default_name) const;

  /// Whether a turbulence Physics has been coupled in, to know which viscosity to pick on symmetry boundary conditions
  bool hasTurbulencePhysics() const
  {
    if (_turbulence_physics)
      return _turbulence_physics->hasTurbulenceModel();
    else
      return false;
  }

  /// Find the turbulence physics
  const WCNSFVTurbulencePhysics * getCoupledTurbulencePhysics() const;

  /// Boolean to keep track of whether the flow equations should be created
  const bool _has_flow_equations;

  /// Compressibility type, can be compressible, incompressible or weakly-compressible
  const MooseEnum _compressibility;

  /// Switch to show if porous medium treatment is requested or not
  const bool _porous_medium_treatment;
  /// The name of the functor for the porosity field
  const MooseFunctorName _porosity_name;
  /// The name of the functor for the smoothed porosity field
  const MooseFunctorName _flow_porosity_functor_name;
  /// The number of smoothing layers if that treatment is used on porosity
  const unsigned _porosity_smoothing_layers;

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
  const std::vector<BoundaryName> _inlet_boundaries;
  /// Boundaries with a flow outlet specified on them
  const std::vector<BoundaryName> _outlet_boundaries;
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

  /// Subdomains where we want to have volumetric friction
  std::vector<std::vector<SubdomainName>> _friction_blocks;
  /// The friction correlation types used for each block
  std::vector<std::vector<std::string>> _friction_types;
  /// The coefficients used for each item if friction type
  std::vector<std::vector<std::string>> _friction_coeffs;
};
