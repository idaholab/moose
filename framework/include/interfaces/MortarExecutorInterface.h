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
 * Interface for objects that need to be notified when the mortar segment mesh for an interface
 * they consume has been (re)built. Notification is push-based: this interface registers itself
 * with the owning \p FEProblemBase's \p MortarInterfaceWarehouse in its constructor, and \p
 * mortarSetup() is called back whenever that warehouse rebuilds an \p
 * AutomaticMortarGeneration and its segment coverage (interior-parent subdomains, boundary
 * materials, etc.) has changed. This replaces re-deriving "has coverage changed" from outside the
 * warehouse on every use.
 */
class MortarExecutorInterface
{
public:
  /**
   * @param fe_problem The problem that owns the \p MortarInterfaceWarehouse this object should
   * register with
   */
  MortarExecutorInterface(FEProblemBase & fe_problem);

  /**
   * Re-registers the moved-to object in place of \p other with the warehouse. Required because
   * consumers such as \p ComputeMortarFunctor are held by value in containers (e.g. \p
   * std::unordered_map) that move-construct on insertion.
   */
  MortarExecutorInterface(MortarExecutorInterface && other);

  /**
   * This object does not deregister from the warehouse on destruction: \p FEProblemBase destroys
   * its \p MortarInterfaceWarehouse (\p _mortar_data) before the solver/auxiliary systems that
   * transitively own long-lived \p MortarExecutorInterface consumers (e.g. \p ComputeMortarFunctor,
   * \p MortarNodalAuxKernelTempl), so deregistering here would touch an already-destroyed
   * warehouse. This mirrors \p MeshChangedInterface, which has the same destruction-order
   * constraint with respect to \p FEProblemBase::_notify_when_mesh_changes. A consumer that is
   * instead stack-constructed fresh per use and destroyed well before \p FEProblemBase (e.g. \p
   * MortarUserObjectThread) must deregister itself in its own destructor, since for such a
   * consumer the warehouse is always still alive and skipping it would leave a dangling entry.
   */
  virtual ~MortarExecutorInterface() = default;

  /**
   * Called by the \p MortarInterfaceWarehouse whenever \p amg's mortar segment mesh coverage has
   * changed, so that this object may refresh any state (e.g. material dependency lists) that
   * depends on which subdomains/boundaries the mortar segment mesh currently touches.
   */
  virtual void mortarSetup(const AutomaticMortarGeneration & amg) = 0;

protected:
  /// The warehouse this object is registered with for mortar setup notifications
  MortarInterfaceWarehouse & _mortar_warehouse;

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
