//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

#include "libmesh/point.h"
#include "libmesh/periodic_boundary_base.h"

#include <array>
#include <vector>

class FEProblemBase;
class Function;

/**
 * Periodic boundary for calculation periodic BC on domains where the
 * translation is given by Function objects
 */
class FunctionPeriodicBoundary : public libMesh::PeriodicBoundaryBase
{
public:
  /**
   * Initialize the periodic with the functions and inverse functions, one
   * for each dimension needed
   */
  FunctionPeriodicBoundary(FEProblemBase & subproblem,
                           const std::vector<std::string> & fn_names,
                           const std::vector<std::string> & inv_fn_names);

  /**
   * Copy constructor for creating the periodic boundary and inverse periodic boundary
   *
   * @param o - Periodic boundary being copied
   * @param t - Transformation direction
   */
  FunctionPeriodicBoundary(const FunctionPeriodicBoundary & o, TransformationType t = FORWARD);

  /**
   * Get the translation based on point 'pt'
   * @param pt - point on the 'source' boundary
   * @return point on the paired boundary
   */
  virtual libMesh::Point get_corresponding_pos(const libMesh::Point & pt) const override;

  virtual std::unique_ptr<libMesh::PeriodicBoundaryBase> clone(TransformationType t) const override;

private:
  static std::array<const Function *, 3> getFunctions(FEProblemBase & problem,
                                                      const std::vector<std::string> & names);

  /// The dimension of the problem (says which of _tr and _inv_tr are active)
  const unsigned int _dim;

  /// Pointers to translation functions in each dimension
  const std::array<const Function *, 3> _tr;
  /// Pointers to inverse translation functions in each dimension
  const std::array<const Function *, 3> _inv_tr;
};
