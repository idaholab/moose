//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsActionBase.h"
#include "libmesh/point.h"

class TensorMechanicsAction : public TensorMechanicsActionBase
{
public:
  static InputParameters validParams();

  TensorMechanicsAction(const InputParameters & params);

  virtual void act();

protected:
  void actSubdomainChecks();
  void actOutputGeneration();
  void actEigenstrainNames();
  void actOutputMatProp();
  void actGatherActionParameters();
  void verifyOrderAndFamilyOutputs();

  virtual std::string getKernelType();
  virtual InputParameters getKernelParameters(std::string type);

  ///@{ displacement variables
  std::vector<VariableName> _displacements;

  /// Number of displacement variables
  unsigned int _ndisp;

  /// Coupled displacement variables
  std::vector<VariableName> _coupled_displacements;
  ///@}

  ///@{ residual debugging
  std::vector<AuxVariableName> _save_in;
  std::vector<AuxVariableName> _diag_save_in;
  ///@}

  Moose::CoordinateSystemType _coord_system;

  /// if this vector is not empty the variables, kernels and materials are restricted to these subdomains
  std::vector<SubdomainName> _subdomain_names;

  /// set generated from the passed in vector of subdomain names
  std::set<SubdomainID> _subdomain_ids;

  /// set generated from the combined block restrictions of all TensorMechanics/Master action blocks
  std::set<SubdomainID> _subdomain_id_union;

  /// strain formulation
  enum class Strain
  {
    Small,
    Finite
  } _strain;

  /// strain formulation
  enum class StrainAndIncrement
  {
    SmallTotal,
    FiniteTotal,
    SmallIncremental,
    FiniteIncremental
  } _strain_and_increment;

  /// use an out of plane stress/strain formulation
  enum class PlanarFormulation
  {
    None,
    WeakPlaneStress,
    PlaneStrain,
    GeneralizedPlaneStrain,
  } _planar_formulation;

  enum class OutOfPlaneDirection
  {
    x,
    y,
    z
  };

  const OutOfPlaneDirection _out_of_plane_direction;

  /// base name for the current master action block
  const std::string _base_name;

  /// use displaced mesh (true unless _strain is SMALL)
  bool _use_displaced_mesh;

  /// output materials to generate scalar stress/strain tensor quantities
  std::vector<std::string> _generate_output;
  std::vector<std::string> _material_output_order;
  std::vector<std::string> _material_output_family;

  /// booleans used to determine if cylindrical axis points are passed
  bool _cylindrical_axis_point1_valid;
  bool _cylindrical_axis_point2_valid;
  bool _direction_valid;
  bool _verbose;

  /// points used to determine axis of rotation for cyclindrical stress/strain quantities
  Point _cylindrical_axis_point1;
  Point _cylindrical_axis_point2;
  Point _direction;

  /// automatically gather names of eigenstrain tensors provided by simulation objects
  const bool _auto_eigenstrain;

  std::vector<MaterialPropertyName> _eigenstrain_names;
};
