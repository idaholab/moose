//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiAppConservativeTransfer.h"

/**
 * Loops over a target mesh and uses either node or element centroid location (based on the target
 * variable type) for sampling a user object (i.e. the object must implement `spatialValue` API).
 * This value is then stored into the target variable (either nodal or elemental).
 * Note: Higher order variables are not supported.
 *
 * This transfer can be block and boundary restricted. The BlockRestrictable and
 * BoundaryRestrictable classes cannot be used, because they would check the block, boundary and
 * target variable during object construction. At that time, the underlying sub-app is not created
 * yet, so this check would fail. That is also the reason why the block and boundary restriction are
 * pulled in during `execute` and not in the constructor. Also note, that in a sub-app setup there
 * might be multiple instances of the sub-app, so this check needs to be done on per-sub-app basis.
 */
class MultiAppUserObjectTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppUserObjectTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * @return true if this transfer is restricted to a block, otherwise false
   */
  bool blockRestricted() const;

  /**
   * @return true if this transfer is restricted to a boundary, otherwise false
   */
  bool boundaryRestricted() const;

  /**
   * Check that element 'elem' is part of the domain this transfer is restricted to
   */
  bool hasBlocks(const Elem * elem) const;

  /**
   * Check that Node 'node' belongs to block this transfer is restricted to
   *
   * @param mesh The mesh this transfer is active on
   * @param node The node to check
   */
  bool hasBlocks(const MooseMesh * mesh, const Node * node) const;

  /**
   * Check that the node belongs to boundary this transfer is restricted to
   *
   * @return true if the node belongs to the boundary this transfer is restricted to, false
   * otherwise
   * @param mesh The mesh this transfer is active on
   * @param node The node to check
   */
  bool isBoundaryNode(const MooseMesh * mesh, const Node * node) const;

  /**
   * Check that the element belongs to boundary this transfer is restricted to
   *
   * @return true if the element belongs to the boundary this transfer is restricted to, false
   * otherwise
   * @param mesh The mesh this transfer is active on
   * @param elem The element to check
   */
  bool isBoundaryElem(const MooseMesh * mesh, const Elem * elem) const;

  /**
   * Gets the UserObject to transfer from when transferring from_multiapp
   * @param p The point in the parent app that is being transferred to
   * @return the subapp index, will return static_cast<unsigned int>(-1) if none is found
   */
  unsigned int findSubAppToTransferFrom(const Point & p);

  std::string _user_object_name;

  /**
   * Boolean variable to generate error if every parent app node
   * cannot be mapped to a subApp during from_multiapp transfer
   **/
  const bool _all_parent_nodes_contained_in_sub_app;

  /// whether to check the bounding box check or not
  const bool _skip_bbox_check;

  /// Whether to utilize the nearest sub-app to transfer from
  const bool & _nearest_sub_app;

private:
  bool usesMooseAppCoordTransform() const override { return true; }

  /// Set of block ids this transfer is restricted to
  std::set<SubdomainID> _blk_ids;

  /// Set of the boundary ids
  std::set<BoundaryID> _bnd_ids;
};
