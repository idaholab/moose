//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

class FEProblemBase;
class MortarData;
class MaterialBase;

/**
 * Interface for notifications that the mortar mesh has been setup
 */
class MortarExecutorInterface
{
public:
  MortarExecutorInterface() = default;

protected:
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
