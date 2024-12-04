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

class NavierStokesPhysicsBase;
class WCNSFVFlowPhysicsBase;
class WCNSFVTurbulencePhysics;

/**
 * Helper class to interact with a flow and turbulence physics for a Physics that solves
 * an advection problem using incompressible or weakly-compressible finite volume
 */
class WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSFVCoupledAdvectionPhysicsHelper(const NavierStokesPhysicsBase * derived_physics);

  const WCNSFVFlowPhysicsBase * getCoupledFlowPhysics() const;
  const WCNSFVTurbulencePhysics * getCoupledTurbulencePhysics() const;

  /// Return the porosity functor name.
  /// It is important to forward to the Physics so we do not get the smoothing status wrong
  MooseFunctorName getPorosityFunctorName(bool smoothed) const;

  const MooseFunctorName & densityName() const { return _density_name; }
  const MooseFunctorName & dynamicViscosityName() const { return _dynamic_viscosity_name; }

protected:
  /// The Physics class using this helper
  const NavierStokesPhysicsBase * _advection_physics;
  /// Flow physics
  const WCNSFVFlowPhysicsBase * _flow_equations_physics;
  /// Turbulence
  const WCNSFVTurbulencePhysics * _turbulence_physics;

  /// Compressibility type, can be compressible, incompressible or weakly-compressible
  const MooseEnum _compressibility;

  /// Switch to show if porous medium treatment is requested or not
  const bool _porous_medium_treatment;

  /// Velocity names
  const std::vector<std::string> _velocity_names;
  /// Pressure name
  const NonlinearVariableName _pressure_name;

  /// Name of the density material property
  const MooseFunctorName _density_name;
  /// Name of the dynamic viscosity material property
  const MooseFunctorName _dynamic_viscosity_name;

  /// The velocity / momentum face interpolation method for advecting other quantities
  const MooseEnum _velocity_interpolation;
};
