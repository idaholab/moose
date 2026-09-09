//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include <map>
#include <deque>
#include <set>

class FEProblemBase;
class MortarInterfaceWarehouse;
class MaterialBase;
class AutomaticMortarGeneration;

/**
 * Interface for notifications that the mortar mesh has been setup
 */
class MortarExecutorInterface
{
public:
  MortarExecutorInterface() = default;

protected:
  /**
   * @return Whether \p _secondary_ip_sub_to_mats or \p _primary_ip_sub_to_mats is missing an entry
   * for a subdomain that \p amg currently reports via \p secondaryIPSubIDs() / \p
   * primaryIPSubIDs(). The mortar segment mesh is rebuilt whenever the (possibly displaced) mesh
   * moves, and a segment only registers its interior-parent subdomain once it has found a valid
   * primary projection
   * (\p AutomaticMortarGeneration::buildMortarSegmentMesh()); an interface whose surfaces do not
   * yet project onto each other therefore registers nothing until relative motion brings it into
   * projection range. If that happens after these maps were last built, \p
   * Moose::Mortar::loopOverMortarSegments will \p libmesh_map_find a subdomain key that was never
   * inserted. That lookup is checked and throws rather than inserting or returning an empty result,
   * so the throw propagates out of residual/Jacobian assembly (e.g.
   * \p FEProblemBase::computeResidualTags) as \p "map_find() error: key ... not found". There is no
   * meaningful way to tolerate a missing key on the spot: the missing deque is precisely the list
   * of face/neighbor materials that must be reinit'd before the consumer's quadrature-point
   * evaluation.
   */
  bool mortarMaterialsNeedSetup(const AutomaticMortarGeneration & amg) const;

  /**
   * @name Materials for Mortar
   * These containers hold the materials whose properties are required by a given set of consumers.
   * Note that these containers will also hold materials that may not provide properties explicitly
   * needed by the \em consumers but do provided properties that are dependencies of the materials
   * that \em do provide properties needed by the consumers
   */
  ///@{
  /// A map from secondary interior parent subdomain IDs to the block materials that will need to
  /// reinit'd on the secondary face
  std::map<SubdomainID, std::deque<MaterialBase *>> _secondary_ip_sub_to_mats;

  /// A map from primary interior parent subdomain IDs to the block materials that will need to
  /// reinit'd on the primary face
  std::map<SubdomainID, std::deque<MaterialBase *>> _primary_ip_sub_to_mats;

  /// A container that holds the boundary materials that will need to be reinit'd on the secondary
  /// face
  std::deque<MaterialBase *> _secondary_boundary_mats;
  ///@}
};
