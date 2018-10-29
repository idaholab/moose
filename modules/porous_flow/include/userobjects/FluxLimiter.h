//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FLUXLIMITER_H
#define FLUXLIMITER_H

#include "GeneralUserObject.h"
#include "Coupleable.h"

/**
 * Does the flux-limiting of KT
 * NOTE: this implementation is only correct for:
 *  - one dimension where the nodes are arranged in numerical order from left to right and the dof
 *    number is equal to the node.id() number
 *  - velocity that is constant over the whole mesh
 **/
class FluxLimiter;

template <>
InputParameters validParams<FluxLimiter>();

class FluxLimiter : public GeneralUserObject, public Coupleable
{
public:
  FluxLimiter(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};

  virtual void timestepSetup() override;
  virtual void meshChanged() override;

  Real val_at_node(const Node & node) const;

  /**
   * Returns the value of R_{i}^{+}, Eqn (49) of KT
   * @param node_i global node number
   */
  Real rPlus(const Node & node_i) const;

  /**
   * Returns the value of R_{i}^{-}, Eqn (49) of KT
   * @param node_i global node number
   */
  Real rMinus(const Node & node_i) const;

  /**
   * Returns the value of k_ij as computed by KT Eqns (18)-(20).
   * @param node_i i^th node
   * @param node_j j^th node
   * @return k_ij of KT
   */
  Real getKij(const Node & node_i, const Node & node_j) const;

  /**
   * flux limiter, L, on Page 135 of Kuzmin and Turek
   */
  Real limitFlux(Real a, Real b) const;

protected:
  /// the nodal values of u
  MooseVariable * _u_nodal;

  /// the moose variable number of u
  unsigned _u_var_num;

  /// Determines Flux Limiter type
  const enum class FluxLimiterTypeEnum { MinMod, VanLeer, MC, superbee, None } _flux_limiter_type;

  /// Reference to the mesh
  const MooseMesh & _mesh;

  /// The data structure used to find neighboring elements given a node ID
  std::vector<std::vector<const Elem *>> _nodes_to_elem_map;

  std::map<Node, std::map<Node, Real>> _kij;

  /**
   * Create _kij and set all its entries to zero
   */
  virtual void initializeKij();

  /**
   * Compute _kij
   */
  virtual void computeKij();

  /**
   * Returns the value of P_{i}^{+}, Eqn (47) of KT
   * @param node_i global node number
   */
  Real pPlus(const Node & node_i) const;

  /**
   * Returns the value of P_{i}^{-}, Eqn (47) of KT
   * @param node_i global node number
   */
  Real pMinus(const Node & node_i) const;

  /**
   * Returns the value of Q_{i}^{+}, Eqn (48) of KT
   * @param node_i global node number
   */
  Real qPlus(const Node & node_i) const;

  /**
   * Returns the value of Q_{i}^{-}, Eqn (48) of KT
   * @param node_i global node number
   */
  Real qMinus(const Node & node_i) const;
};

#endif // FLUXLIMITER_H
