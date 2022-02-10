//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedGeneralUserObject.h"

/**
 * Abstract base class for computing and caching internal or boundary fluxes for RDG
 * for the 3-equation model of 1-phase flow.
 *
 * This base class adds interfaces useful for testing purposes; specifically,
 * since some fluxes have different regions, depending on flow conditions, it
 * is important to ensure that all are tested. To help with this, an interface
 * is added to retrieve the last entered region.
 */
class NumericalFlux3EqnBase : public ThreadedGeneralUserObject
{
public:
  NumericalFlux3EqnBase(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject &) override;

  /**
   * Gets the flux vector for an element/side combination
   *
   * If the element/side is cached, the cached values are used. Otherwise the
   * values are computed, cached, and returned.
   *
   * @param[in] iside    local index of current side
   * @param[in] ielem    global index of the current element
   * @param[in] res_side_is_left    getting flux on the left ("elem") side?
   * @param[in] UL       vector of variables on the "left"
   * @param[in] UR       vector of variables on the "right"
   * @param[in] nLR_dot_d   Dot product of direction from "left" to "right" with
   *                        the flow channel direction
   *
   * @return flux vector for an element/side combination
   */
  virtual const std::vector<Real> & getFlux(const unsigned int iside,
                                            const dof_id_type ielem,
                                            bool res_side_is_left,
                                            const std::vector<Real> & UL,
                                            const std::vector<Real> & UR,
                                            const Real & nLR_dot_d) const;

  /**
   * Gets the flux Jacobian matrix for an element/side combination
   *
   * If the element/side is cached, the cached values are used. Otherwise the
   * values are computed, cached, and returned. The user decides which Jacobian
   * to receive (either with respect to "left" solution vector or "right" solution
   * vector). If the user wants both, then the function can just be called a
   * second time with the other Jacobian requested - since that Jacobian is
   * already cached, no redundant calculations will occur.
   *
   * @param[in] res_side_is_left    getting flux on the left ("elem") side?
   * @param[in] jac_side_is_left    taking derivative w.r.t. the left ("elem") side?
   * @param[in] iside    local index of current side
   * @param[in] ielem    global index of the current element
   * @param[in] UL       vector of variables on the "left"
   * @param[in] UR       vector of variables on the "right"
   * @param[in] nLR_dot_d   Dot product of direction from "left" to "right" with
   *                        the flow channel direction
   *
   * @return flux Jacobian matrix for an element/side combination
   */
  virtual const DenseMatrix<Real> & getJacobian(bool res_side_is_left,
                                                bool jac_side_is_left,
                                                const unsigned int iside,
                                                const dof_id_type ielem,
                                                const std::vector<Real> & UL,
                                                const std::vector<Real> & UR,
                                                const Real & nLR_dot_d) const;

  /**
   * Calculates the flux vectors given "left" and "right" states
   *
   * This function is called only if the values are not already cached.
   *
   * @param[in] UL        vector of variables on the "left"
   * @param[in] UR        vector of variables on the "right"
   * @param[in] nLR_dot_d   Dot product of direction from "left" to "right" with
   *                        the flow channel direction
   * @param[out] FL       flux vector to be added to "left" side
   * @param[out] FR       flux vector to be added to "right" side
   */
  virtual void calcFlux(const std::vector<Real> & UL,
                        const std::vector<Real> & UR,
                        const Real & nLR_dot_d,
                        std::vector<Real> & FL,
                        std::vector<Real> & FR) const = 0;

  /**
   * Calculates the flux Jacobian matrices given "left" and "right" states
   *
   * This function is called only if the values are not already cached.
   *
   * @param[in] UL        vector of variables on the "left"
   * @param[in] UR        vector of variables on the "right"
   * @param[in] nLR_dot_d   Dot product of direction from "left" to "right" with
   *                        the flow channel direction
   * @param[out] dFL_dUL   Jacobian matrix of "left" flux w.r.t. "left" solution.
   * @param[out] dFL_dUR   Jacobian matrix of "left" flux w.r.t. "right" solution.
   * @param[out] dFR_dUL   Jacobian matrix of "right" flux w.r.t. "left" solution.
   * @param[out] dFR_dUR   Jacobian matrix of "right" flux w.r.t. "right" solution.
   */
  virtual void calcJacobian(const std::vector<Real> & UL,
                            const std::vector<Real> & UR,
                            const Real & nLR_dot_d,
                            DenseMatrix<Real> & dFL_dUL,
                            DenseMatrix<Real> & dFL_dUR,
                            DenseMatrix<Real> & dFR_dUL,
                            DenseMatrix<Real> & dFR_dUR) const = 0;

  /**
   * Returns the index of the region last entered
   *
   * Here "region" refers to a code path taken. For some fluxes, such as centered
   * fluxes, there is just a single code path, but for others, such as those
   * using an approximate Riemann solver, there are multiple. Riemann solvers
   * have "regions" defined by the characteristic waves.
   */
  unsigned int getLastRegionIndex() const { return _last_region_index; }

  /**
   * Returns the total possible number of regions
   *
   * Here "region" refers to a code path taken. For some fluxes, such as centered
   * fluxes, there is just a single code path, but for others, such as those
   * using an approximate Riemann solver, there are multiple. Riemann solvers
   * have "regions" defined by the characteristic waves.
   */
  virtual unsigned int getNumberOfRegions() const = 0;

protected:
  /// element ID of the cached flux values
  mutable unsigned int _cached_flux_elem_id;
  /// side ID of the cached flux values
  mutable unsigned int _cached_flux_side_id;

  /// element ID of the cached Jacobian values
  mutable unsigned int _cached_jacobian_elem_id;
  /// side ID of the cached Jacobian values
  mutable unsigned int _cached_jacobian_side_id;

  /// flux vector for the "left" cell
  mutable std::vector<Real> _FL;
  /// flux vector for the "right" cell
  mutable std::vector<Real> _FR;
  /// derivative of "left" flux w.r.t. "left" solution
  mutable DenseMatrix<Real> _dFL_dUL;
  /// derivative of "left" flux w.r.t. "right" solution
  mutable DenseMatrix<Real> _dFL_dUR;
  /// derivative of "right" flux w.r.t. "left" solution
  mutable DenseMatrix<Real> _dFR_dUL;
  /// derivative of "right" flux w.r.t. "right" solution
  mutable DenseMatrix<Real> _dFR_dUR;

  /// Index describing the region last entered, which is useful for testing and debugging
  mutable unsigned int _last_region_index;

public:
  static InputParameters validParams();
};
