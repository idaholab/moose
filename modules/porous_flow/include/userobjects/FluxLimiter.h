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
  virtual void execute() override{};
  virtual void finalize() override{};

  Real val_at_node(const Node & node) const;

  /**
   * Returns the value of R_{i}^{+}, Eqn (49) of KT
   * @param node_i global node number
   */
  Real rPlus(unsigned node_i) const;

  /**
   * Returns the value of R_{i}^{-}, Eqn (49) of KT
   * @param node_i global node number
   */
  Real rMinus(unsigned node_i) const;

  /**
   * Returns the value of P_{i}^{+}, Eqn (47) of KT
   * @param node_i global node number
   */
  Real pPlus(unsigned node_i) const;

  /**
   * Returns the value of P_{i}^{-}, Eqn (47) of KT
   * @param node_i global node number
   */
  Real pMinus(unsigned node_i) const;

  /**
   * Returns the value of Q_{i}^{+}, Eqn (48) of KT
   * @param node_i global node number
   */
  Real qPlus(unsigned node_i) const;

  /**
   * Returns the value of Q_{i}^{-}, Eqn (48) of KT
   * @param node_i global node number
   */
  Real qMinus(unsigned node_i) const;

  /**
   * Returns the value of k_ij / velocity as computed
   * by KT Eqns (18)-(20).
   * @param node_i global node number
   * @param node_j global node number
   * @return k_ij of KT
   */
  Real kij(unsigned node_i, unsigned node_j) const;

  /**
   * flux limiter, L, on Page 135 of Kuzmin and Turek
   */
  Real fluxLimiter(Real a, Real b) const;

protected:
  /// the nodal values of u
  MooseVariable * _u_nodal;

  /// the moose variable number of u
  unsigned _u_var_num;

  /// Determines Flux Limiter type
  const enum class FluxLimiterTypeEnum { MinMod, VanLeer, MC, superbee, None } _flux_limiter_type;
};

#endif // FLUXLIMITER_H
