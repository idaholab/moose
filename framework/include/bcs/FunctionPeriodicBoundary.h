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

#include "Function.h"

#include "point.h"
#include "periodic_boundaries.h"

class MooseSystem;

/**
 * Periodic boundary for calculation periodic BC on domains where the translation is given by functions
 */
class FunctionPeriodicBoundary : public PeriodicBoundary
{
public:

  /**
   * Initialize the periodic boundary with three function
   */
  FunctionPeriodicBoundary(MooseSystem &moose_system, std::vector<std::string> fn_names);
  
  /**
   * Copy constructor for creating the periodic boundary and inverse periodic boundary
   *
   * @param o - Periodic boundary being copied
   * @param inverse - true if creating the inverse periodic boundary
   */
  FunctionPeriodicBoundary(const FunctionPeriodicBoundary & o, bool inverse = false);

  /**
   * Get the translation based on point 'pt'
   * @param pt - point on the 'source' boundary
   * @return point on the paired boundary
   */
  virtual Point get_corresponding_pos(const Point & pt);

protected:
  int _dim;
  Real _dir;

  Function & _tr_x;
  Function & _tr_y;
  Function & _tr_z;
};

#endif //FUNCTIONPERIODICBOUNDARY_H
