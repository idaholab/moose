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
 * A base class for computing and caching internal side flux
 *
 * Notes:
 *
 *   1. When solving a system of equations, fluxes are threated as vectors of all variables.
 *      To avoid recomputing the flux for each equation, we compute it once and store it.
 *      Then, when the flux is needed by another equation,
 *      this class just returns the cached value.
 *
 *   2. Derived classes need to provide computing of the fluxes and their jacobians,
 *      i.e., they need to implement `calcFlux` and `calcJacobian`.
 */
class InternalSideFluxBase : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  InternalSideFluxBase(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject &) override;

  /**
   * Get the flux vector
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   ineig     global index of the neighbor element
   * @param[in]   uvec1     vector of variables on the "left"
   * @param[in]   uvec2     vector of variables on the "right"
   * @param[in]   dwave     vector of unit normal
   */
  virtual const std::vector<Real> & getFlux(unsigned int iside,
                                            dof_id_type ielem,
                                            dof_id_type ineig,
                                            const std::vector<Real> & uvec1,
                                            const std::vector<Real> & uvec2,
                                            const RealVectorValue & dwave) const;

  /**
   * Solve the Riemann problem
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   ineig     global index of the neighbor element
   * @param[in]   uvec1     vector of variables on the "left"
   * @param[in]   uvec2     vector of variables on the "right"
   * @param[in]   dwave     vector of unit normal
   * @param[out]  flux      flux vector across the side
   */
  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        dof_id_type ineig,
                        const std::vector<Real> & uvec1,
                        const std::vector<Real> & uvec2,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const = 0;

  /**
   * Get the Jacobian matrix
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   ineig     global index of the neighbor element
   * @param[in]   uvec1     vector of variables on the "left"
   * @param[in]   uvec2     vector of variables on the "right"
   * @param[in]   dwave     vector of unit normal
   */
  virtual const DenseMatrix<Real> & getJacobian(Moose::DGResidualType type,
                                                unsigned int iside,
                                                dof_id_type ielem,
                                                dof_id_type ineig,
                                                const std::vector<Real> & uvec1,
                                                const std::vector<Real> & uvec2,
                                                const RealVectorValue & dwave) const;

  /**
   * Compute the Jacobian matrix
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   ineig     global index of the neighbor element
   * @param[in]   uvec1     vector of variables on the "left"
   * @param[in]   uvec2     vector of variables on the "right"
   * @param[in]   dwave     vector of unit normal
   * @param[out]  jac1      Jacobian matrix contribution to the "left" cell
   * @param[out]  jac2      Jacobian matrix contribution to the "right" cell
   */
  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            dof_id_type ineig,
                            const std::vector<Real> & uvec1,
                            const std::vector<Real> & uvec2,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const = 0;

protected:
  /// element ID of the cached flux values
  mutable unsigned int _cached_flux_elem_id;
  /// neighbor element ID of the cached flux values
  mutable unsigned int _cached_flux_neig_id;

  /// element ID of the cached Jacobian values
  mutable unsigned int _cached_jacobian_elem_id;
  /// neighbor element ID of the cached Jacobian values
  mutable unsigned int _cached_jacobian_neig_id;

  /// flux vector of this side
  mutable std::vector<Real> _flux;
  /// Jacobian matrix contribution to the "left" cell
  mutable DenseMatrix<Real> _jac1;
  /// Jacobian matrix contribution to the "right" cell
  mutable DenseMatrix<Real> _jac2;
};
