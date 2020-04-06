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

class LineElementAction : public Action
{
public:
  static InputParameters validParams();

  LineElementAction(const InputParameters & params);

  virtual void act();

  /// Add parameters required for a beam element
  static InputParameters beamParameters();

protected:
  /**
   * Gather all the block ids from all the actions of this type to create
   * variables spanning all the blocks
   **/
  void actGatherActionParameters();

  /// Adds displacement and rotation variables
  void actAddVariables();

  /// Adds material objects required for beam and truss elements
  void actAddMaterials();

  /**
   * Adds StressDivergence kernels for beam and truss elements and inertia
   * kernels for dynamic beam simulations
   **/
  void actAddKernels();

  /**
   * Adds translational and rotational velocity and acceleration aux variables
   * for dynamic beam simulations
   **/
  void actAddAuxVariables();

  /**
   * Adds auxkernels corresponding to the translational and rotational velocity
   * and acceleration aux variables
   **/
  void actAddAuxKernels();

  /**
   * Adds nodal kernels that calculate inertial force/torque due to mass/inertia
   * assigned to nodes of the beam
   **/
  void actAddNodalKernels();

  /// Names of displacement variables
  std::vector<VariableName> _displacements;

  /// Number of displacement variables
  unsigned int _ndisp;

  /// Names of rotational variables for beam element
  std::vector<VariableName> _rotations;

  /**
   * Names of translational velocity variables for dynamic simulation using
   * beam element
   **/
  std::vector<VariableName> _velocities;

  /**
   * Names of translational acceleration variables for dynamic simulation
   * beam element
   **/
  std::vector<VariableName> _accelerations;

  /**
   * Names of rotational velocity variables for dynamic simulation using
   * beam element
   **/
  std::vector<VariableName> _rot_velocities;

  /**
   * Names of rotational acceleration variables for dynamic simulation
   * beam element
   **/
  std::vector<VariableName> _rot_accelerations;

  ///@{ residual debugging
  std::vector<AuxVariableName> _save_in;
  std::vector<AuxVariableName> _diag_save_in;
  ///@}

  /**
   * If this vector is not empty the variables, auxvariables, kernels,
   * auxkernels, nodalkernels and materials are restricted to these subdomains
   **/
  std::vector<SubdomainName> _subdomain_names;

  /// set generated from the passed in vector of subdomain names
  std::set<SubdomainID> _subdomain_ids;

  /**
   * set generated from the combined block restrictions of all LineElementAction
   * action blocks
   **/
  std::set<SubdomainID> _subdomain_id_union;

  /// strain formulation
  enum class Strain
  {
    SMALL,
    FINITE
  } _strain_type;

  Strain _rotation_type;

  /// use displaced mesh (true unless _strain is SMALL_STRAIN_AND_ROTATION)
  bool _use_displaced_mesh;

  /**
   * Set to true to set up translational and acceleration AuxVariables and
   * the corresponding AuxKernels using the action when the dynamic kernels
   * or nodal kernels are not set by the action.
   **/
  bool _add_dynamic_variables;

  /**
   * Set to true to use consistent mass and inertia matrices to calculate
   * inertial forces/torques in dynamic beam simulations
   **/
  bool _dynamic_consistent_inertia;

  /**
   * Set to true to use nodal mass matrix to calculate inertial forces in
   * dynamic beam simulations
   **/
  bool _dynamic_nodal_translational_inertia;

  /**
   * Set to true to use nodal inertia matrix to calculate inertial torques
   * in dynamic beam simulations
   **/
  bool _dynamic_nodal_rotational_inertia;

  /// Set to true if line element is a truss
  bool _truss;
};
