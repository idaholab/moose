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

#include <set>

class InputParameters;
class MooseObject;
class FEProblemBase;
class MooseMesh;
class MortarData;

/**
 * An interface for accessing mortar mesh data
 *
 * \note moi is shorthand for mortar interface, so \p _moi_problem indicates
 *       the mortar interface's problem
 */
class MortarInterface
{
public:
  MortarInterface(const MooseObject * moose_object);

  static InputParameters validParams();

  /**
   * Returns the master lower dimensional subdomain id
   */
  SubdomainID masterSubdomain() const { return _master_subdomain_id; }

protected:
  const std::set<SubdomainID> & getHigherDimSubdomainIDs() const
  {
    return _higher_dim_subdomain_ids;
  }
  const std::set<BoundaryID> & getBoundaryIDs() const { return _boundary_ids; }

private:
  FEProblemBase & _moi_problem;

  /// This mesh corresponds to the reference space mesh always, but
  /// we are just querying it for ID information which should be
  /// the exact same on reference and displaced meshes
  MooseMesh & _moi_mesh;

  /// A reference to the mortar data object that holds all the mortar
  /// mesh information
  const MortarData & _mortar_data;

  /// Boundary ID for the secondary surface
  const BoundaryID _secondary_id;

  /// Boundary ID for the master surface
  const BoundaryID _master_id;

  /// Subdomain ID for the secondary surface
  const SubdomainID _secondary_subdomain_id;

  /// Subdomain ID for the master surface
  const SubdomainID _master_subdomain_id;

  /// the union of the secondary and master boundary ids
  std::set<BoundaryID> _boundary_ids;

  /// the higher dimensional subdomain ids corresponding to the interior parents
  std::set<SubdomainID> _higher_dim_subdomain_ids;
};
