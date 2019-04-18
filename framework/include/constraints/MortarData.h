#ifndef MORTARDATA_H
#define MORTARDATA_H

#include "AutomaticMortarGeneration.h"

#include <map>

class SubProblem;

class MortarData
{
public:
  MortarData();

  /**
   * Getter to retrieve the AutomaticMortarGeneration object corresponding to the boundary and
   * subdomain keys. If the AutomaticMortarGeneration object does not yet exist, then it is created
   * using the mesh of the passed in subproblem
   */
  AutomaticMortarGeneration &
  getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                     const std::pair<SubdomainID, SubdomainID> & subdomain_key,
                     SubProblem & subproblem,
                     bool on_displaced);

  /**
   * Getter to retrieve the AutomaticMortarGeneration object corresponding to the boundary and
   * subdomain keys. If the AutomaticMortarGeneration object does not yet exist, then we error
   */
  AutomaticMortarGeneration &
  getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                     const std::pair<SubdomainID, SubdomainID> & /*subdomain_key*/);

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
  bool hasDisplacedObjects() const { return _has_displaced_objects; }

  /**
   * Returns whether we have **any** active AutomaticMortarGeneration objects
   */
  bool hasObjects() const { return _mortar_interfaces.size(); }

protected:
  /// Map from master-slave (in that order) boundary ID pair to the corresponding
  /// AutomaticMortarGeneration object
  std::map<std::pair<BoundaryID, BoundaryID>, std::unique_ptr<AutomaticMortarGeneration>>
      _mortar_interfaces;

  /// A set containing the subdomain ids covered by all the mortar interfaces in this MortarData
  /// object
  std::set<SubdomainID> _mortar_subdomain_coverage;

  /// A set containing the boundary ids covered by all the mortar interfaces in this MortarData
  /// object
  std::set<BoundaryID> _mortar_boundary_coverage;

  /// Whether any of the AutomaticMortarGeneration objects are running on a displaced mesh
  bool _has_displaced_objects;
};

#endif
