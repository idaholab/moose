//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AutomaticMortarGeneration.h"
#include "MooseHashing.h"

#include "libmesh/parallel_object.h"

#include <unordered_map>
#include <set>

class SubProblem;

class MortarData : public libMesh::ParallelObject
{
public:
  MortarData(const libMesh::ParallelObject & other);

  /**
   * Create mortar generation object
   * @param boundary_key The master-secondary boundary pair on which the AMG objects lives
   * @param subdomain_key The master-secondary subdomain pair on which the AMG objects lives
   * @param subproblem A reference to the subproblem
   * @param on_displaced Whether the AMG object lives on the displaced mesh
   * @param periodic Whether the AMG object will be used for enforcing periodic constraints. Note
   * that this changes the direction of the projection normals so one AMG object cannot be used to
   * enforce both periodic and non-periodic constraints
   */
  void createMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                             const std::pair<SubdomainID, SubdomainID> & subdomain_key,
                             SubProblem & subproblem,
                             bool on_displaced,
                             bool periodic);

  /**
   * Getter to retrieve the AutomaticMortarGeneration object corresponding to the boundary and
   * subdomain keys. If the AutomaticMortarGeneration object does not yet exist, then we error
   */
  const AutomaticMortarGeneration &
  getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                     const std::pair<SubdomainID, SubdomainID> & /*subdomain_key*/,
                     bool on_displaced) const;

  /**
   * Return all automatic mortar generation objects on either the displaced or undisplaced mesh
   */
  const std::unordered_map<std::pair<BoundaryID, BoundaryID>, AutomaticMortarGeneration> &
  getMortarInterfaces(bool on_displaced) const
  {
    if (on_displaced)
      return _displaced_mortar_interfaces;
    else
      return _mortar_interfaces;
  }

  /**
   * Returns the mortar covered subdomains
   */
  const std::set<SubdomainID> & getMortarSubdomainIDs() const { return _mortar_subdomain_coverage; }

  /**
   * Returns the mortar covered boundaries
   */
  const std::set<BoundaryID> & getMortarBoundaryIDs() const { return _mortar_boundary_coverage; }

  /**
   * Builds mortar segment meshes for each mortar interface
   */
  void update();

  /**
   * Returns whether any of the AutomaticMortarGeneration objects are running on a displaced mesh
   */
  bool hasDisplacedObjects() const { return _displaced_mortar_interfaces.size(); }

  /**
   * Returns whether we have **any** active AutomaticMortarGeneration objects
   */
  bool hasObjects() const { return _mortar_interfaces.size(); }

  /**
   * Returns the higher dimensional subdomain ids of the interior parents of the given lower-d
   * subdomain id
   */
  const std::set<SubdomainID> & getHigherDimSubdomainIDs(SubdomainID lower_d_subdomain_id) const;

private:
  /**
   * Builds mortar segment mesh from specific AutomaticMortarGeneration object
   */
  void update(AutomaticMortarGeneration & amg);

  typedef std::pair<BoundaryID, BoundaryID> MortarKey;

  /// Map from master-secondary (in that order) boundary ID pair to the corresponding
  /// undisplaced AutomaticMortarGeneration object
  std::unordered_map<MortarKey, AutomaticMortarGeneration> _mortar_interfaces;

  /// Map from master-secondary (in that order) boundary ID pair to the corresponding
  /// displaced AutomaticMortarGeneration object
  std::unordered_map<MortarKey, AutomaticMortarGeneration> _displaced_mortar_interfaces;

  /// A set containing the subdomain ids covered by all the mortar interfaces in this MortarData
  /// object
  std::set<SubdomainID> _mortar_subdomain_coverage;

  /// A set containing the boundary ids covered by all the mortar interfaces in this MortarData
  /// object
  std::set<BoundaryID> _mortar_boundary_coverage;

  /// Map from undisplaced AMG key to whether the undisplaced AMG object is enforcing periodic constraints
  std::unordered_map<MortarKey, bool> _periodic_map;

  /// Map from displaced AMG key to whether the displaced AMG object is enforcing periodic constraints
  std::unordered_map<MortarKey, bool> _displaced_periodic_map;

  /// Map from lower dimensional subdomain ids to corresponding higher simensional subdomain ids
  /// (e.g. the ids of the interior parents)
  std::unordered_map<SubdomainID, std::set<SubdomainID>> _lower_d_sub_to_higher_d_subs;
};
