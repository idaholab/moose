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
class ADNumericalFlux3EqnBase : public ThreadedGeneralUserObject
{
public:
  ADNumericalFlux3EqnBase(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject &) override;

  /**
   * Gets the 1D flux vector for an element/side combination
   *
   * If the element/side is cached, the cached values are used. Otherwise the
   * values are computed, cached, and returned.
   *
   * @param[in] iside    local index of current side
   * @param[in] ielem    global index of the current element
   * @param[in] res_side_is_left    getting flux on the left ("elem") side?
   * @param[in] UL_1d    vector of 1D flux inputs on the "left"
   * @param[in] UR_1d    vector of 1D flux inputs on the "right"
   * @param[in] nLR_dot_d   Dot product of direction from "left" to "right" with
   *                        the flow channel direction
   *
   * @return flux vector for an element/side combination
   */
  virtual const std::vector<ADReal> & getFlux(const unsigned int iside,
                                              const dof_id_type ielem,
                                              bool res_side_is_left,
                                              const std::vector<ADReal> & UL_1d,
                                              const std::vector<ADReal> & UR_1d,
                                              const ADReal & nLR_dot_d) const;

  /**
   * Gets the 3D flux vector for an element/side combination
   *
   * If the element/side is cached, the cached values are used. Otherwise the
   * values are computed, cached, and returned.
   *
   * @param[in] iside    local index of current side
   * @param[in] ielem    global index of the current element
   * @param[in] res_side_is_left    getting flux on the left ("elem") side?
   * @param[in] UL_3d    vector of 3D flux inputs on the "left"
   * @param[in] UR_3d    vector of 3D flux inputs on the "right"
   * @param[in] nLR      Direction from "left" to "right"
   * @param[in] t1       1st tangent direction
   * @param[in] t2       2nd tangent direction
   *
   * @return flux vector for an element/side combination
   */
  virtual const std::vector<ADReal> & getFlux3D(const unsigned int iside,
                                                const dof_id_type ielem,
                                                bool res_side_is_left,
                                                const std::vector<ADReal> & UL_3d,
                                                const std::vector<ADReal> & UR_3d,
                                                const RealVectorValue & nLR,
                                                const RealVectorValue & t1,
                                                const RealVectorValue & t2) const;

  /**
   * Calculates the 3D flux vectors given "left" and "right" states
   *
   * This function is called only if the values are not already cached.
   *
   * @param[in] UL_3d  Vector of 3D flux inputs on the "left"
   * @param[in] UR_3d  Vector of 3D flux inputs on the "right"
   * @param[in] nLR   Direction from "left" to "right"
   * @param[in] t1    1st tangent direction
   * @param[in] t2    2nd tangent direction
   * @param[out] FL   Flux vector to be added to "left" side
   * @param[out] FR   Flux vector to be added to "right" side
   */
  virtual void calcFlux(const std::vector<ADReal> & UL_3d,
                        const std::vector<ADReal> & UR_3d,
                        const RealVectorValue & nLR,
                        const RealVectorValue & t1,
                        const RealVectorValue & t2,
                        std::vector<ADReal> & FL,
                        std::vector<ADReal> & FR) const = 0;

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

  /// flux vector for the "left" cell for 3D
  mutable std::vector<ADReal> _FL_3d;
  /// flux vector for the "right" cell for 3D
  mutable std::vector<ADReal> _FR_3d;
  /// flux vector for the "left" cell for 1D
  mutable std::vector<ADReal> _FL_1d;
  /// flux vector for the "right" cell for 1D
  mutable std::vector<ADReal> _FR_1d;

  /// Index describing the region last entered, which is useful for testing and debugging
  mutable unsigned int _last_region_index;

public:
  static InputParameters validParams();
};
