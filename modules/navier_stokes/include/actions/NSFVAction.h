//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

#include "MooseEnum.h"
#include "libmesh/fe_type.h"

/**
 * This class allows us to have a section of the input file like the
 * following which automatically adds variables, kernels, aux kernels, bcs
 * for setting up the incompressible Navier-Stokes equation.
 *
 * [FiniteVolumeNavierStokes]
 * []
 */
class NSFVAction : public Action
{
public:
  static InputParameters validParams();

  NSFVAction(InputParameters parameters);

  virtual void act() override;

protected:
  void addINSTimeKernels();
  void addINSMass();
  void addINSMomentum();
  void addINSEnergy();
  void addINSInletBC();
  void addINSOutletBC();
  void addINSWallBC();

  void addWCNSTimeKernels();
  void addWCNSEnergy();
  void addWCNSVelocityBC();
  void addWCNSInletBC();
  void addWCNSOutletBC();
  void addWCNSWallBC();

  void addCNSTimeKernels();
  void addCNSMass();
  void addCNSMomentum();
  void addCNSEnergy();
  void addCNSInletBC();
  void addCNSOutletBC();
  void addCNSWallBC();

  /// Equation type, transient or steady-state
  MooseEnum _type;

  /// Compressibility type, can be compressible, incompressible
  /// or weakly-incompressible
  MooseEnum _compressibility;
  /// Swich dedicated to show if porous medium treatment is requested or not
  bool _porous_medium_treatment;
  /// The name of the auxiliary variable for the porosity field
  AuxVariableName _porosity_name;
  /// Turbulent diffusivity handling type (mixing-length, etc.)
  MooseEnum _turbulence_handling;

  /// Subdomains Navier-Stokes equation is defined on
  std::vector<SubdomainName> _blocks;
  /// Subdomain IDs
  std::set<SubdomainID> _block_ids;

  /// Boundaries with a flow inlet specified on them
  std::vector<BoundaryName> _inlet_boundaries;
  /// Boundaries with a flow outlet specified on them
  std::vector<BoundaryName> _outlet_boundaries;
  /// Boundaries which define a wall (slip/noslip/etc.)
  std::vector<BoundaryName> _wall_boundaries;

  /// Velocity intlet types (fixed-velocity/mass-flow/momentum-inflow)
  MultiMooseEnum _momentum_inlet_types;
  /// Velocity function names at velocity inlet boundaries
  std::vector<FunctionName> _momentum_inlet_function;
  /// Velocity outlet types (pressure/mass-outflow/momentum-outflow)
  MultiMooseEnum _momentum_outlet_types;
  /// Velocity wall types (symmetry/noslip/slip/wallfunction)
  MultiMooseEnum _momentum_wall_types;

  /// Energy intlet types (fixed-velocity/mass-flow/momentum-inflow)
  MultiMooseEnum _energy_inlet_types;
  /// Energy function names at inlet boundaries
  std::vector<FunctionName> _energy_inlet_function;
  /// Energy wall types (symmetry/heatflux/fixed-temperature)
  MultiMooseEnum _energy_wall_types;
  /// Energy function names at wall boundaries
  std::vector<FunctionName> _energy_wall_function;

  /// Boundaries with pressure specified
  std::vector<BoundaryName> _pressure_boundary;
  /// Pressure function names at pressure boundaries
  std::vector<FunctionName> _pressure_function;
  /// Whether or not we need to pin pressure at a node
  bool _has_pinned_dof;
  /// The node set name of the pinned node
  BoundaryName _pinned_dof;

  /// Boundaries with temperature specified
  std::vector<BoundaryName> _fixed_temperature_boundary;
  /// Temperature function names at fixed temperature boundaries
  std::vector<FunctionName> _temperature_function;

  /// Temperature variable name in the solid subscale structure (in porous medium treatment)
  /// to facilitate temperature variable added outside
  VariableName _solid_temperature_variable_name;
  /// Mesh dimension
  unsigned int _dim;

  /// Name of the density material property
  MaterialPropertyName _density_name;
  /// Name of the dynamic viscosity material property
  MaterialPropertyName _dynamic_viscosity_name;
  /// Name of the specific heat material property
  MaterialPropertyName _specific_heat_name;
  /// Name of the thermal conductivity material property
  MaterialPropertyName _thermal_conductivity_name;
  /// NAme of the thermal expansion material property
  MaterialPropertyName _thermal_expansion_name;

  /// Type that we use in Actions for declaring coupling
  typedef std::vector<VariableName> CoupledName;
};
