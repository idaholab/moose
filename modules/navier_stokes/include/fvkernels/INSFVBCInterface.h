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
#include "INSFVAttributes.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "TheWarehouse.h"
#include "MooseApp.h"
#include "INSFVNoSlipWallBC.h"
#include "INSFVSlipWallBC.h"
#include "INSFVSymmetryBC.h"
#include "INSFVFullyDevelopedFlowBC.h"
#include <set>

/**
 * This interface gives the inheriting class information about all the different boundary conditions
 * that surround a flow physics region. A use case for this interface is forcing execution of
 * advection kernels on flow boundaries
 */
class INSFVBCInterface
{
protected:
  /**
   * setup all the boundary condition member information
   */
  template <typename T>
  void initialSetup(T & insfv_fk);

  /// Boundary IDs with no slip walls
  std::set<BoundaryID> _no_slip_wall_boundaries;

  /// Boundary IDs with slip walls
  std::set<BoundaryID> _slip_wall_boundaries;

  /// Flow Boundary IDs
  std::set<BoundaryID> _flow_boundaries;

  /// Fully Developed Flow Boundary IDs. This is a subset of \p _flow_boundaries
  std::set<BoundaryID> _fully_developed_flow_boundaries;

  /// Symmetry Boundary IDs
  std::set<BoundaryID> _symmetry_boundaries;

  /// All the BoundaryIDs covered by our different types of INSFVBCs
  std::set<BoundaryID> _all_boundaries;

private:
  /**
   * Query for \p INSFVBCs::INSFVFlowBC on \p bc_id and add if query successful
   */
  template <typename T>
  void setupFlowBoundaries(T & insfv_fk, BoundaryID bnd_id);

  /**
   * Query for \p INSFVBCs on \p bc_id and add if query successful
   */
  template <typename T, typename T2>
  void setupBoundaries(T2 & insfv_fk,
                       const BoundaryID bnd_id,
                       INSFVBCs bc_type,
                       std::set<BoundaryID> & bnd_ids);
};

template <typename T>
void
INSFVBCInterface::initialSetup(T & insfv_fk)
{
  const auto & mesh = insfv_fk.subProblem().mesh();

  std::set<BoundaryID> all_connected_boundaries;
  const auto & blk_ids = insfv_fk.blockRestricted() ? insfv_fk.blockIDs() : mesh.meshSubdomains();
  for (const auto blk_id : blk_ids)
  {
    const auto & connected_boundaries = mesh.getSubdomainBoundaryIds(blk_id);
    for (const auto bnd_id : connected_boundaries)
      all_connected_boundaries.insert(bnd_id);
  }

  for (const auto bnd_id : all_connected_boundaries)
  {
    setupFlowBoundaries(insfv_fk, bnd_id);
    setupBoundaries<INSFVNoSlipWallBC>(
        insfv_fk, bnd_id, INSFVBCs::INSFVNoSlipWallBC, _no_slip_wall_boundaries);
    setupBoundaries<INSFVSlipWallBC>(
        insfv_fk, bnd_id, INSFVBCs::INSFVSlipWallBC, _slip_wall_boundaries);
    setupBoundaries<INSFVSymmetryBC>(
        insfv_fk, bnd_id, INSFVBCs::INSFVSymmetryBC, _symmetry_boundaries);
  }
}

template <typename T>
void
INSFVBCInterface::setupFlowBoundaries(T & insfv_fk, const BoundaryID bnd_id)
{
  std::vector<INSFVFlowBC *> flow_bcs;

  insfv_fk.subProblem()
      .getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribBoundaries>(bnd_id)
      .template condition<AttribINSFVBCs>(INSFVBCs::INSFVFlowBC)
      .queryInto(flow_bcs);

  if (!flow_bcs.empty())
  {
    if (dynamic_cast<INSFVFullyDevelopedFlowBC *>(flow_bcs.front()))
    {
      _fully_developed_flow_boundaries.insert(bnd_id);

#ifndef NDEBUG
      for (auto * flow_bc : flow_bcs)
        mooseAssert(dynamic_cast<INSFVFullyDevelopedFlowBC *>(flow_bc),
                    "If one BC is a fully developed flow BC, then all other flow BCs on that "
                    "boundary must also be fully developed flow BCs");
    }
    else
      for (auto * flow_bc : flow_bcs)
        mooseAssert(!dynamic_cast<INSFVFullyDevelopedFlowBC *>(flow_bc),
                    "If one BC is not a fully developed flow BC, then all other flow BCs on that "
                    "boundary must also not be fully developed flow BCs");
#else
    }
#endif

    _flow_boundaries.insert(bnd_id);
    _all_boundaries.insert(bnd_id);
  }
}

template <typename T, typename T2>
void
INSFVBCInterface::setupBoundaries(T2 & insfv_fk,
                                  const BoundaryID bnd_id,
                                  const INSFVBCs bc_type,
                                  std::set<BoundaryID> & bnd_ids)
{
  std::vector<T *> bcs;

  insfv_fk.subProblem()
      .getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribBoundaries>(bnd_id)
      .template condition<AttribINSFVBCs>(bc_type)
      .queryInto(bcs);

  if (!bcs.empty())
  {
    bnd_ids.insert(bnd_id);
    _all_boundaries.insert(bnd_id);
  }
}
