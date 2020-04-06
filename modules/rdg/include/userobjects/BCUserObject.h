//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * A base class of user object for calculating
 * the variable values in ghost element
 * according to specific boundary conditions
 *
 * Notes:
 *
 *   1. In CFD, a ghost element means a virtual element that is
 *      outside a mesh and shares a common boundary face
 *      with an element in the mesh.
 *
 *   2. This user object is used in two places.
 *
 *      First, it is used in a slope reconstruction user object,
 *      in which a slope reconstruction scheme in an element
 *      adjacent to a boundary face requires adequate boundary conditions
 *      to ensure globally second-order accurate in space.
 *
 *      Second, it is used in a boundary flux user object,
 *      where boundary conditions are required to properly calculate
 *      the flux vector and Jacobian matrix across the boundary face.
 *
 *   3. Derived classes have to override `getGhostCellValue`.
 */
class BCUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  BCUserObject(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

  /**
   * compute the ghost cell variable values
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   uvec1     vector of variables on the host side
   * @param[in]   dwave     vector of unit normal
   */
  virtual std::vector<Real> getGhostCellValue(unsigned int iside,
                                              dof_id_type ielem,
                                              const std::vector<Real> & uvec1,
                                              const RealVectorValue & dwave) const = 0;
};
