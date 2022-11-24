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
 * [IncompressibleNavierStokes]
 * []
 */
class INSAction : public Action
{
public:
  static InputParameters validParams();

  INSAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  // Helper function that sets the parameters which are common to all INS Kernels.
  void setKernelCommonParams(InputParameters & params);
  void setNoBCCommonParams(InputParameters & params);

  // Helper functions that add various inviscid flux Kernels.
  void addINSTimeKernels();
  void addINSMass();
  void addINSMomentum();
  void addINSTemperature();
  void addINSVelocityAux();

  // Helper functions that add BCs.
  void addINSVelocityBC();
  void addINSPinnedPressureBC();
  void addINSNoBCBC();
  void addINSPressureBC();
  void addINSTemperatureBC();

  /// Equation type, transient or steady-state
  MooseEnum _type;
  /// Subdomains Navier-Stokes equation is defined on
  std::vector<SubdomainName> _blocks;
  /// Boundaries with velocity specified
  std::vector<BoundaryName> _velocity_boundary;
  /// Velocity function names at velocity boundaries
  std::vector<FunctionName> _velocity_function;
  /// Boundaries with pressure specified
  std::vector<BoundaryName> _pressure_boundary;
  /// Pressure function names at pressure boundaries
  std::vector<FunctionName> _pressure_function;
  /// No-BC boundaries
  std::vector<BoundaryName> _no_bc_boundary;
  /// Whether or not we need to pin pressure at a node
  bool _has_pinned_node;
  /// The node set name of the pinned node
  BoundaryName _pinned_node;
  /// Boundaries with temperature specified
  std::vector<BoundaryName> _fixed_temperature_boundary;
  /// Temperature function names at fixed temperature boundaries
  std::vector<FunctionName> _temperature_function;
  /// FE type for various variables
  FEType _fe_type;
  /// Whether we use AD or not
  bool _use_ad;
  /// Temperature variable name to facilitate temperature variable added outside
  VariableName _temperature_variable_name;
  /// Mesh dimension
  unsigned int _dim;
  /// Subdomain IDs
  std::set<SubdomainID> _block_ids;
  /// pressure variable name
  const std::string _pressure_variable_name;

  /// Type that we use in Actions for declaring coupling
  typedef std::vector<VariableName> CoupledName;
};
