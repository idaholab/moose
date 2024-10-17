//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

// libMesh
#include "libmesh/point.h"
#include "libmesh/periodic_boundary_base.h"

#include <vector>

class FEProblemBase;
class Function;

/**
 * Periodic boundary for calculation periodic BC on domains where the translation is given by
 * functions
 */
class FunctionPeriodicBoundary : public libMesh::PeriodicBoundaryBase
{
public:
  /**
   * Initialize the periodic boundary with three functions
   */
  FunctionPeriodicBoundary(FEProblemBase & subproblem, std::vector<std::string> fn_names);

  /**
   * Copy constructor for creating the periodic boundary and inverse periodic boundary
   *
   * @param o - Periodic boundary being copied
   */
  FunctionPeriodicBoundary(const FunctionPeriodicBoundary & o);

  /**
   * Get the translation based on point 'pt'
   * @param pt - point on the 'source' boundary
   * @return point on the paired boundary
   */
  virtual libMesh::Point get_corresponding_pos(const libMesh::Point & pt) const override;

  /**
   * Required interface, this class must be able to clone itself
   */
  virtual std::unique_ptr<libMesh::PeriodicBoundaryBase> clone(TransformationType t) const override;

protected:
  //  /// The dimension of the problem (says which _tr_XYZ member variables are active)
  unsigned int _dim;

  /// Pointer to Function for x-component of the boundary
  const Function * const _tr_x;

  /// Pointer to Function for y-component of the boundary
  const Function * const _tr_y;

  /// Pointer to Function for z-component of the boundary
  const Function * const _tr_z;

  /**
   * An initialization method to make certain that initialSetup() of a function prior to value()
   */
  void init();
};
