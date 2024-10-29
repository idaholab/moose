/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "MultiAppConservativeTransfer.h"

class MultiAppUserObjectTransferSCM;

/**
 * This is a copy of MultiAppUserObjectTransfer, but swaps y-, and z- coordinate, so we can
 * map the subchannel solution on a RZ-mesh of a heating pin.
 *
 * This should be considered as a temporary solution until a proper capability is built in MOOSE
 *
 * NOTE: This will need a better name before merging into devel
 */
class MultiAppUserObjectTransferSCM : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppUserObjectTransferSCM(const InputParameters & parameters);

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
   * Get application bounding box.
   */
  BoundingBox getBoundingBox(unsigned int app, bool displaced_mesh);

  std::string _user_object_name;

  /**
   * Boolean variable to generate error if every master node
   * cannot be mapped to a subApp during from_multiapp transfer
   **/
  const bool _all_master_nodes_contained_in_sub_app;

  /// whether to check the bounding box check or not
  const bool _skip_bbox_check;

private:
  /// Set of block ids this transfer is restricted to
  std::set<SubdomainID> _blk_ids;

  /// Set of the boundary ids
  std::set<BoundaryID> _bnd_ids;
};
