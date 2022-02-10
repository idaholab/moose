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
 * A base class for computing/caching fluxes at boundaries
 *
 * Notes:
 *
 *   1. When systems of equations are being solved, the fluxes are treated as vectors.
 *      To avoid recomputing the flux at the boundary, we compute it just once
 *      and then when it is needed, we just return the cached value.
 *
 *   2. Derived classes need to override `calcFlux`.
 */
class ADBoundaryFluxBase : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  ADBoundaryFluxBase(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject &) override;

  /**
   * Get the boundary flux vector
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   uvec1     vector of variables on the host side
   * @param[in]   dwave     vector of unit normal
   */
  virtual const std::vector<ADReal> & getFlux(unsigned int iside,
                                              dof_id_type ielem,
                                              const std::vector<ADReal> & uvec1,
                                              const RealVectorValue & dwave) const;

  /**
   * Solve the Riemann problem on the boundary face
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   uvec1     vector of variables on the host side
   * @param[in]   dwave     vector of unit normal
   * @param[out]  flux      flux vector for conservation equations
   */
  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<ADReal> & uvec1,
                        const RealVectorValue & dwave,
                        std::vector<ADReal> & flux) const = 0;

protected:
  /// element ID of the cached flux values
  mutable unsigned int _cached_flux_elem_id;
  /// side ID of the cached flux values
  mutable unsigned int _cached_flux_side_id;

  /// Cached flux
  mutable std::vector<ADReal> _flux;
};
