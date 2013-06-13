/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef FUNCTIONPERIODICBOUNDARY_H
#define FUNCTIONPERIODICBOUNDARY_H

#include "Moose.h"

//libMesh
#include "libmesh/point.h"
#include "libmesh/periodic_boundary_base.h"

#include <vector>

class FEProblem;
class Function;

/**
 * Periodic boundary for calculation periodic BC on domains where the translation is given by functions
 */
class FunctionPeriodicBoundary : public PeriodicBoundaryBase
{
public:

  /**
   * Initialize the periodic boundary with three functions
   */
  FunctionPeriodicBoundary(FEProblem & subproblem, std::vector<std::string> fn_names);

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
  virtual Point get_corresponding_pos(const Point & pt) const;

  /**
   * Required interface, this class must be able to clone itself
   */
  virtual AutoPtr<PeriodicBoundaryBase> clone(TransformationType t) const;

protected:
  /// The dimension of the problem (says which _tr_XYZ member variables are active)
  unsigned int _dim;

  /// Pointer to Function for x-component of the boundary
  Function * _tr_x;

  /// Pointer to Function for y-component of the boundary
  Function * _tr_y;

  /// Pointer to Function for z-component of the boundary
  Function * _tr_z;

  /**
   * An initialization method to make certain that intialSetup() of a function prior to value()
   */
  void init();
};

#endif //FUNCTIONPERIODICBOUNDARY_H
