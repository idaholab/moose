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
#include "libmesh/quadrature_gauss.h"

namespace libMesh
{
class QGauss;
}

class ShellElementAction : public Action
{
public:
  static InputParameters validParams();

  ShellElementAction(const InputParameters & params);

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

  /// Names of displacement variables
  std::vector<VariableName> _displacements;

  /// Number of displacement variables
  unsigned int _ndisp;

  /// Names of rotational variables for beam element
  std::vector<VariableName> _rotations;

  /// Number of rotation variables
  unsigned int _nrot;

  /// Quadrature rule in the out of plane direction
  std::unique_ptr<QGauss> _t_qrule;

  /// Number of Gauss points considered through the thickness
  unsigned int _ntpoints;

  /// Names of rotational variables for beam element
  VariableName _thickness;

  /// Flag to compute large strains
  const bool _large_strain;

  /// strain formulation
  enum class Strain
  {
    SMALL,
    FINITE
  } _strain_type;

  Strain _rotation_type;

  /// Check if simulation uses incremental strain increments
  bool _incremental;

  /**
   * If this vector is not empty the variables, auxvariables, kernels,
   * auxkernels, nodalkernels and materials are restricted to these subdomains
   **/
  std::vector<SubdomainName> _subdomain_names;

  /// set generated from the passed in vector of subdomain names
  std::set<SubdomainID> _subdomain_ids;

  /**
   * set generated from the combined block restrictions of all ShellElementAction
   * action blocks
   **/
  std::set<SubdomainID> _subdomain_id_union;

  /// use displaced mesh (true unless _strain is SMALL_STRAIN_AND_ROTATION)
  bool _use_displaced_mesh;

  ///@{ residual debugging
  std::vector<AuxVariableName> _save_in;
  std::vector<AuxVariableName> _diag_save_in;
  ///@}

};
