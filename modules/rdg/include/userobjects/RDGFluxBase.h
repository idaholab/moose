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
 *
 * Here a call to compute flux computes the flux for all equations in the system,
 * so to avoid duplicating the calculations, a wrapper is used to cache the
 * system flux for an element/side combination.
 */
class RDGFluxBase : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  RDGFluxBase(const InputParameters & parameters);

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
   * @param[in] uvec1    vector of variables on the "left"
   * @param[in] uvec2    vector of variables on the "right"
   * @param[in] normal   vector of unit normal
   *
   * @return flux vector for an element/side combination
   */
  virtual const std::vector<Real> & getFlux(const unsigned int iside,
                                            const dof_id_type ielem,
                                            const std::vector<Real> & uvec1,
                                            const std::vector<Real> & uvec2,
                                            const RealVectorValue & normal) const;

  /**
   * Gets the flux Jacobian matrix for an element/side combination
   *
   * If the element/side is cached, the cached values are used. Otherwise the
   * values are computed, cached, and returned. The user decides which Jacobian
   * to receive (either with respect to first solution vector or second solution
   * vector). If the user wants both, then the function can just be called a
   * second time with the other Jacobian requested - since that Jacobian is
   * already cached, no redundant calculations will occur.
   *
   * @param[in] get_first_jacobian   get the Jacobian w.r.t. the first solution;
   *                                 else get the Jacobian w.r.t. the second solution
   * @param[in] iside    local index of current side
   * @param[in] ielem    global index of the current element
   * @param[in] uvec1    vector of variables on the "left"
   * @param[in] uvec2    vector of variables on the "right"
   * @param[in] normal   vector of unit normal
   *
   * @return flux Jacobian matrix for an element/side combination
   */
  virtual const DenseMatrix<Real> & getJacobian(const bool get_first_jacobian,
                                                const unsigned int iside,
                                                const dof_id_type ielem,
                                                const std::vector<Real> & uvec1,
                                                const std::vector<Real> & uvec2,
                                                const RealVectorValue & normal) const;

  /**
   * Calculates the flux vector given "left" and "right" states
   *
   * This function is called only if the values are not already cached.
   *
   * @param[in]  uvec1    vector of variables on the "left"
   * @param[in]  uvec2    vector of variables on the "right"
   * @param[in]  normal   vector of unit normal
   * @param[out] flux     flux vector
   */
  virtual void calcFlux(const std::vector<Real> & uvec1,
                        const std::vector<Real> & uvec2,
                        const RealVectorValue & normal,
                        std::vector<Real> & flux) const = 0;

  /**
   * Calculates the flux Jacobian matrices given "left" and "right" states
   *
   * This function is called only if the values are not already cached.
   *
   * @param[in]  uvec1    vector of variables on the "left"
   * @param[in]  uvec2    vector of variables on the "right"
   * @param[in]  normal   vector of unit normal
   * @param[out] jac1     Jacobian matrix contribution to the "left" cell
   * @param[out] jac2     Jacobian matrix contribution to the "right" cell
   */
  virtual void calcJacobian(const std::vector<Real> & uvec1,
                            const std::vector<Real> & uvec2,
                            const RealVectorValue & normal,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const = 0;

protected:
  /// element ID of the cached flux values
  mutable unsigned int _cached_flux_elem_id;
  /// side ID of the cached flux values
  mutable unsigned int _cached_flux_side_id;

  /// element ID of the cached Jacobian values
  mutable unsigned int _cached_jacobian_elem_id;
  /// side ID of the cached Jacobian values
  mutable unsigned int _cached_jacobian_side_id;

  /// flux vector
  mutable std::vector<Real> _flux;
  /// Jacobian matrix contribution to the "left" cell
  mutable DenseMatrix<Real> _jac1;
  /// Jacobian matrix contribution to the "right" cell
  mutable DenseMatrix<Real> _jac2;
};
