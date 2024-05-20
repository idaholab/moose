//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * Update side sets around subdomains during a run, as subdomain IDs are changing.
 * This object will have to be used with the execution_order_group parameter to ensure
 * element subdomain IDs are fully updated in the mesh before this object runs.
 */
class SidesetAroundSubdomainUpdater : public DomainUserObject
{
public:
  static InputParameters validParams();

  SidesetAroundSubdomainUpdater(const InputParameters & parameters);

  virtual void initialize() override;

  virtual void executeOnExternalSide(const Elem * /*elem*/, unsigned int /*side*/) override;
  virtual void executeOnInternalSide() override;

  virtual void finalize() override;

  virtual void threadJoin(const UserObject &) override;

protected:
  void processSide(const Elem * primary_elem,
                   unsigned short int primary_side,
                   const Elem * secondary_elem);

  /// The MPI rank of this processor
  const processor_id_type _pid;

  /// Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

  /// Current side on the neighboring element
  const unsigned int & _neighbor_side;

  ///@{ Subdomains on the two sides of the boundary
  std::set<SubdomainID> _inner_ids;
  std::set<SubdomainID> _outer_ids;
  ///@}

  /// assign sideset to sides that have no neighbor elements
  const bool _assign_outer_surface_sides;

  ///@{ Boundary / sideset to update
  BoundaryName _boundary_name;
  BoundaryID _boundary_id;
  ///@}

  BoundaryInfo & _boundary_info;
  BoundaryInfo * _displaced_boundary_info;

  using SideList = std::vector<std::tuple<dof_id_type, unsigned short int>>;

  std::map<processor_id_type, SideList> _add, _remove;
};
