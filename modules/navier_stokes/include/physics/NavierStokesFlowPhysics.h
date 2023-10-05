//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"

/**
 * Creates all the objects needed to solve the Navier Stokes equations
 */
class NavierStokesFlowPhysics : public PhysicsBase
{
public:
  static InputParameters validParams();

  NavierStokesFlowPhysics(const InputParameters & parameters);

  /// GeneralUO not the right base class probably
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

  /// To interface with other Physics
  std::vector<std::string> getVelocityNames() const { return _velocity_names; }
  NonlinearVariableName getPressureName() const { return _pressure_name; }
  NonlinearVariableName getTemperatureName() const { return _fluid_temperature_name; }
  MooseFunctorName getPorosityFunctorName(bool smoothed) const;

protected:
  /// To interface with components
  void processMesh();

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

  /// Boundaries with a flow inlet specified on them
  const std::vector<BoundaryName> _inlet_boundaries;
  /// Boundaries with a flow outlet specified on them
  const std::vector<BoundaryName> _outlet_boundaries;
  /// Boundaries which define a wall (slip/noslip/etc.)
  const std::vector<BoundaryName> _wall_boundaries;

  /// Name of the density material property
  const MooseFunctorName _density_name;
  /// Name of the dynamic viscosity material property
  const MooseFunctorName _dynamic_viscosity_name;
};
