//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVPhysicsBase.h"

class WCNSFVTurbulencePhysics;

/**
 * Creates all the objects needed to solve the Navier Stokes mass and momentum equations
 */
class WCNSFVFlowPhysics final : public WCNSFVPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVFlowPhysics(const InputParameters & parameters);

  /// Whether the physics is actually creating the flow equations
  bool hasFlowEquations() const { return _has_flow_equations; }

  // Getters to interact with other WCNSFVPhysics classes
  /// Get the face interpolation method for velocity
  const MooseEnum & getVelocityInterpolationMethod() const { return _velocity_interpolation; }
  /// Get the inlet direction if using a flux inlet
  const std::vector<Point> & getFluxInletDirections() const { return _flux_inlet_directions; }
  /// Get the inlet flux postprocessor if using a flux inlet
  const std::vector<PostprocessorName> & getFluxInletPPs() const { return _flux_inlet_pps; }
  /// Return the name of the Rhie Chow user object
  std::string rhieChowUOName() const;
  /// Return the number of algebraic ghosting layers needed
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

protected:
private:
  void addNonlinearVariables() override;
  void addInitialConditions() override;
  void addFVKernels() override;
  void addFVBCs() override;
  void addMaterials() override;
  void addUserObjects() override;
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

  /// Add material to define the local speed in porous medium flows
  void addPorousMediumSpeedMaterial();

  /// Function which adds the RhieChow interpolator user objects for weakly and incompressible formulations
  void addRhieChowUserObjects();
  /// Checks that sufficient Rhie Chow coefficients have been defined for the given dimension, used
  /// for scalar or temperature advection by auxiliary variables
  void checkRhieChowFunctorsDefined() const;

  /// Convenience routine to be able to retrieve the actual variable names from their default names
  VariableName getFlowVariableName(const std::string & default_name) const;

  /// Whether a turbulence Physics has been coupled in, to know which viscosity to pick on symmetry boundary conditions
  bool hasTurbulencePhysics() const { return !(!_turbulence_physics); }

  /// Boolean to keep track of whether the flow equations should be created
  const bool _has_flow_equations;

  /// The velocity / momentum face interpolation method for advecting other quantities
  const MooseEnum _velocity_interpolation;

  /// Can be set to a coupled turbulence physics
  const WCNSFVTurbulencePhysics * _turbulence_physics;

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
