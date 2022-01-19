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
  // Helper function that sets the parameters which are common to all INS Kernels.
  void setKernelCommonParams(InputParameters & params);
  void setNoBCCommonParams(InputParameters & params);

  // Helper functions that add various inviscid flux Kernels.
  void addINSTimeKernels();
  void addINSMass();
  void addINSMomentum();
  void addINSEnergy();
  void addINSVelocityBC();
  void addINSInletVelocityBC(unsigned int bc_index);
  void addINSOutletVelocityBC(unsigned int bc_index);
  void addINSWallVelocityBC(unsigned int bc_index);
  void addINSPinnedPressureBC();
  void addINSPressureBC();
  void addINSTemperatureBC();

  void addWCNSTimeKernels();
  void addWCNSEnergy();
  void addWCNSVelocityBC();
  void addWCNSPinnedPressureBC();
  void addWCNSPressureBC();
  void addWCNSTemperatureBC();

  void addCNSTimeKernels();
  void addCNSMass();
  void addCNSMomentum();
  void addCNSEnergy();
  void addCNSVelocityBC();
  void addCNSPinnedPressureBC();
  void addCNSPressureBC();
  void addCNSTemperatureBC();

  /// Equation type, transient or steady-state
  MooseEnum _type;

  /// Compressibility type, can be compressible, incompressible
  /// or weakly-incompressible
  MooseEnum _compressibility;
  /// Swich dedicated to show if porous medium treatment is requested or not
  bool _porous_medium_treatment;
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

  /// Velocity function names at velocity inlet boundaries
  std::vector<FunctionName> _velocity_inlet_function;
  /// Velocity outlet types (pressure/mass-outflow/momentum-outflow)
  MultiMooseEnum _velocity_outlet_types;
  /// Velocity wall types (symmetry/noslip/slip/wallfunction)
  MultiMooseEnum _velocity_wall_types;

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
  /// Temperature variable name to facilitate temperature variable added outside
  VariableName _fluid_temperature_variable_name;
  /// Temperature variable name in the solid subscale structure (in porous medium treatment)
  /// to facilitate temperature variable added outside
  VariableName _solid_temperature_variable_name;
  /// Mesh dimension
  unsigned int _dim;

  /// pressure variable name
  const std::string _pressure_variable_name;

  /// Type that we use in Actions for declaring coupling
  typedef std::vector<VariableName> CoupledName;
};
