//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADVECTIVEFLUXCALCULATOR_H
#define ADVECTIVEFLUXCALCULATOR_H

#include "ElementLoopUserObject.h"

/**
 * Computes K_ij and R+ and R- detailed in
 * D Kuzmin and S Turek "High-resolution FEM-TVD schemes based on a fully multidimensional flux
 * limiter" Journal of Computational Physics 198 (2004) 131-158
 *
 * K_ij is a measure of flux from node i to node j
 * R^+_i and R^-_i quantify how much antidiffusion to allow around node i
 **/
class AdvectiveFluxCalculator;

template <>
InputParameters validParams<AdvectiveFluxCalculator>();

class AdvectiveFluxCalculator : public ElementLoopUserObject
{
public:
  AdvectiveFluxCalculator(const InputParameters & parameters);

  /// Create the _nodes_to_elem_map, if needed
  virtual void timestepSetup() override;

  /// Call ElementLoopUserObject::meshChanged() and then recreate the _nodes_to_elem_map
  virtual void meshChanged() override;

  /// Create a zeroed _kij
  virtual void pre() override;

  /// Compute contributions to _kij from the current element
  virtual void computeElement() override;

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
  /// advection velocity
  RealVectorValue _velocity;

  /// the nodal values of u
  MooseVariable * _u_nodal;

  /// the moose variable number of u
  unsigned _u_var_num;

  /// Kuzmin-Turek shape function
  const VariablePhiValue & _phi;

  /// grad(Kuzmin-Turek shape function)
  const VariablePhiGradient & _grad_phi;

  /**
   * Determines Flux Limiter type (Page 135 of Kuzmin and Turek)
   * "None" means that limitFlux=0 always, which implies zero antidiffusion will be added
   */
  const enum class FluxLimiterTypeEnum { MinMod, VanLeer, MC, superbee, None } _flux_limiter_type;

  /// The data structure used to find neighboring elements given a node ID
  std::vector<std::vector<const Elem *>> _nodes_to_elem_map;

  /// Kuzmin-Turek K_ij matrix.
  std::map<Node, std::map<Node, Real>> _kij;

  enum class PQPlusMinusEnum
  {
    PPlus,
    PMinus,
    QPlus,
    QMinus
  };

  /**
   * Returns the value of P_{i}^{+}, P_{i}^{-}, Q_{i}^{+} or Q_{i}^{-} (depending on pq_plus_minus)
   * which are defined in Eqns (47) and (48) of KT
   * @param node_i global node number
   * @param pq_plus_minus indicates whether P_{i}^{+}, P_{i}^{-}, Q_{i}^{+} or Q_{i}^{-} should be
   * returned
   */
  Real PQPlusMinus(const Node & node_i, const PQPlusMinusEnum pq_plus_minus) const;
};

#endif // ADVECTIVEFLUXCALCULATOR_H
