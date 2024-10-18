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
#include "libmesh/vector_value.h"

/**
 * This class allows us to have a section of the input file like the
 * following which automatically adds variables, kernels, aux kernels, bcs
 * for setting up the Navier-Stokes equation.
 *
 * [Modules]
 *   [CompressibleNavierStokes]
 *   []
 * []
 */
class CNSAction : public Action
{
public:
  static InputParameters validParams();

  CNSAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  // Helper function that sets the parameters which are common to all NSKernels.
  void setKernelCommonParams(InputParameters & params);

  // Helper function that sets the parameters which are common to all NSBCs.
  void setBCCommonParams(InputParameters & params);
  void setStagnationBCCommonParams(InputParameters & params, unsigned int i);

  // Couple the appropriate number (depending on the _dim) of velocity/momentum
  // components into a Kernel.
  void coupleVelocities(InputParameters & params);
  void coupleMomentums(InputParameters & params);

  // Helper functions that add various inviscid flux Kernels.
  void addNSTimeKernels();
  void addNSMassInviscidFlux();
  void addNSMomentumInviscidFlux(unsigned int component);
  void addNSEnergyInviscidFlux();

  // Helper functions that add SUPG Kernels
  void addNSSUPGMass();
  void addNSSUPGMomentum(unsigned int component);
  void addNSSUPGEnergy();

  // Helper functions that add AuxKernels
  void addPressureOrTemperatureAux(const std::string & kernel_type);
  void addNSVelocityAux(unsigned int component);
  void addSpecificTotalEnthalpyAux();
  void addNSMachAux();
  void addNSInternalEnergyAux();
  void addSpecificVolumeComputation();

  // Helper functions that add the various weak stagnation BCs.
  void addNSMassWeakStagnationBC();
  void addNSEnergyWeakStagnationBC();
  void addNSMomentumWeakStagnationBC(unsigned int component);

  // Helper function that adds the no-penetration BCs
  void addNoPenetrationBC(unsigned int component);

  // Helper function that adds the static pressure outlet BCs
  void addNSMassUnspecifiedNormalFlowBC();
  void addNSEnergyInviscidSpecifiedPressureBC();
  void addNSMomentumInviscidSpecifiedPressureBC(unsigned int component);

  /// Equation type, transient or steady-state
  MooseEnum _type;
  /// Name of the FluidProperties object to pass on to Kernels
  UserObjectName _fp_name;
  /// Subdomains Navier-Stokes equation is defined on
  std::vector<SubdomainName> _blocks;
  /// Boundaries stagnation BC applies
  std::vector<BoundaryName> _stagnation_boundary;
  /// Pressures on stagnation boundaries
  std::vector<Real> _stagnation_pressure;
  /// Temperatures on stagnation boundaries
  std::vector<Real> _stagnation_temperature;
  /// Flow directions on stagnation boundaries
  std::vector<Real> _stagnation_direction;
  /// Boundaries no-penetration BC applies
  std::vector<BoundaryName> _no_penetration_boundary;
  /// Boundaries static pressure BC applies
  std::vector<BoundaryName> _static_pressure_boundary;
  /// Pressures on static pressure boundaries
  std::vector<Real> _static_pressure;
  /// FE type for various variables
  libMesh::FEType _fe_type;
  /// Initial value for pressure
  Real _initial_pressure;
  /// Initial value for temperature
  Real _initial_temperature;
  /// Initial value for velocity
  RealVectorValue _initial_velocity;
  /// Mesh dimension
  unsigned int _dim;
  /// Subdomain IDs
  std::set<SubdomainID> _block_ids;
  /// pressure variable name
  const std::string _pressure_variable_name;
};
