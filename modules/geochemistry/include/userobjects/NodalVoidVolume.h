//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

/**
 * Computes the void volume associated with each node.  This is
 * sum_{elem} sum_{qp} _JxW[qp] * _coord[qp] * _phi[node][qp] * porosity[qp]
 * where sum_{elem} runs over all elements connected to the node.
 *
 * It is important to evaluate this properly for nodes on the boundary of a processor's domain, so
 * there is some MPI communication involved.
 * - A relationship manager ensures that this UserObject knows about 1 layer of ghosted elements:
 * all elements that share a node with the elements owned by this processor are known about by this
 * UserObject
 * - We record information about all nodes in the owned + ghosted elements
 * - The execute() method loops over all elements owned by this processor computing the above sum,
 * but not the ghosted elements.
 * - The threadJoin() method adds together all contributions from all threads.  Hence, after
 * threadJoin, the nodal void volume is correct for all nodes that are totally within the region
 * owned by this processor.
 * - The finalize() method calls exchangeGhostedInfo() which adds nodal void volume calculated by
 * other processors to the _nodal_void_volume held by this processor.  Hence, _nodal_void_volume
 * will be correct for all nodes within and on the boundary of this processor's domain.  (The
 * ghosted nodes won't have the correct _nodal_void_volume, but those are never queried by an
 * AuxKernel.)
 */
class NodalVoidVolume : public ElementUserObject
{
public:
  static InputParameters validParams();
  NodalVoidVolume(const InputParameters & parameters);

  /// Zeroes _nodal_void_volume
  virtual void initialize() override;

  /**
   * Loops over all elements owned by this processor, computing the sum specified above.
   */
  virtual void execute() override;

  /**
   * Adds _nodal_void_volume from all threads that have looped over different elements in their
   * execute().  After calling this function, _nodal_void_volume will be correct for nodes that are
   * internal to this processor's domain
   */
  virtual void threadJoin(const UserObject & uo) override;

  /// If there are more than 1 processor, call exchangeGhostedInfo
  virtual void finalize() override;

  /// Set _rebuilding_needed = true to signal that the internal datastructures need rebuilding
  virtual void meshChanged() override;

  /// If _rebuilding_needed then rebuildStructures()
  virtual void timestepSetup() override;
  virtual void initialSetup() override;

  /*
   * @return the void volume
   * @param node_id the node ID
   */
  Real getNodalVoidVolume(const Node * node) const;

private:
  /// porosity
  const VariableValue & _porosity;

  /**
   * whether reinitializing of _nodal_void_volume and rebuilding MPI communication lists is needed,
   * for instance after the mesh has changed
   */
  bool _rebuilding_needed;

  /**
   * _nodal_void_volume[node] = void volume of the node.  The purpose of this UserObject is to
   * compute _nodal_void_volume
   */
  std::unordered_map<const Node *, Real> _nodal_void_volume;

  /// processor ID of this object
  const processor_id_type _my_pid;

  /**
   * _nodes_to_receive[proc_id] = list of my nodes.  proc_id will send us _nodal_void_volume
   * at those nodes. _nodes_to_receive is built (in buildCommLists()) using global node IDs, but
   * after construction, a translation to Node pointers is made.
   * The result is: we will receive _nodal_void_volume[_nodes_to_receive[proc_id][:]] from proc_id
   */
  std::map<processor_id_type, std::vector<const Node *>> _nodes_to_receive;

  /**
   * _nodes_to_send[proc_id] = list of my nodes.  We will send _nodal_void_volume at those
   * nodes to proc_id.  _nodes_to_send is built (in buildCommLists()) using global node IDs, but
   * after construction, a translation to my Node pointers is made.  The result is: we
   * will send _nodal_void_volume[_nodes_to_receive[proc_id][:]] to proc_id
   */
  std::map<processor_id_type, std::vector<const Node *>> _nodes_to_send;

  /// shape function
  const VariablePhiValue & _phi;

  /**
   * reinitialize _nodal_void_volume and rebuild MPI communication lists of this object
   */
  void rebuildStructures();

  /**
   * Build MPI communication lists specifying which _nodal_void_volume info should be exchanged with
   * other processors
   */
  void buildCommLists();

  /**
   * Exchange _nodal_void_volume for nodes that are joined to elements owned by other processors.
   * After calling this function, _nodal_void_volume will be correct for all nodes within and on the
   * boundary of this processor's domain.
   */
  void exchangeGhostedInfo();
};
