//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DomainUserObject.h"

class DisplacedProblem;

/**
 * Promotes fully-damaged CZM interface segments into pressure sidesets at runtime
 */
class CZMPressureSidesetUpdater : public DomainUserObject
{
public:
  static InputParameters validParams();
  CZMPressureSidesetUpdater(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void executeOnElement() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  using SideList = std::vector<std::pair<dof_id_type, unsigned int>>;

  /// True if damage >= threshold at every QP of the current lower-d element.
  bool currentElemFullySeparated() const;

  /// Build _secondary_to_primary_faces from a mortar projection of the undisplaced interface.
  void buildReferencePrimaryFaceMap();

  /// CZM damage (0 = intact, 1 = broken), valid on the secondary lower-d block.
  const VariableValue & _damage;

  /// Damage at/above which a segment is fully separated (effectively 1.0).
  const Real _threshold;

  /// The mortar UO name (to retrieve the interface's AutomaticMortarGeneration).
  const UserObjectName _czm_userobject_name;

  /// Primary/secondary interface boundary ids (to key the mortar interface).
  BoundaryID _primary_boundary_id;
  BoundaryID _secondary_boundary_id;

  /// Primary/secondary lower-dimensional subdomain ids (to key the mortar interface).
  SubdomainID _primary_subdomain_id;
  SubdomainID _secondary_subdomain_id;

  /// Displaced problem
  DisplacedProblem * _displaced_problem;

  ///@{ Destination pressure boundaries.
  BoundaryName _pressure_name_primary;
  BoundaryName _pressure_name_secondary;
  BoundaryID _pressure_bid_primary;
  BoundaryID _pressure_bid_secondary;
  ///@}

  BoundaryInfo & _boundary_info;
  BoundaryInfo * _displaced_boundary_info;

  /**
   * Secondary lower-d element id -> every primary (elem id, side) whose face overlaps it in the
   * reference configuration. Built on thread 0 only; it is consulted in finalize().
   */
  std::unordered_map<dof_id_type, SideList> _secondary_to_primary_faces;

  /// Thread-local fully-separated secondary lower-d element ids found this execution.
  std::vector<dof_id_type> _separated_lowerd;
};
