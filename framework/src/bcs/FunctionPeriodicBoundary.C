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

#include "FunctionPeriodicBoundary.h"
#include "SubProblem.h"

/**
 */
FunctionPeriodicBoundary::FunctionPeriodicBoundary(SubProblem & subproblem, std::vector<std::string> fn_names) :
    _dim(fn_names.size()),
    _dir(1.0),
    _tr_x(&subproblem.getFunction(fn_names[0])),
    _tr_y(fn_names.size() > 1 ? &subproblem.getFunction(fn_names[1]) : NULL),
    _tr_z(fn_names.size() > 2 ? &subproblem.getFunction(fn_names[2]) : NULL)
{
}

FunctionPeriodicBoundary::FunctionPeriodicBoundary(const FunctionPeriodicBoundary & o, bool inverse/* = false*/) :
    PeriodicBoundary(o, inverse),
    _dim(o._dim),
    _dir(inverse ? -1.0 : 1.0),
    _tr_x(o._tr_x),
    _tr_y(o._tr_y),
    _tr_z(o._tr_z)
{
}

Point
FunctionPeriodicBoundary::get_corresponding_pos(const Point & pt) const
{
  Real t = 0.;
  Point p;
  switch (_dim)
  {
  case 1:
    return Point(_tr_x->value(t, pt));

  case 2:
    return Point(_tr_x->value(t, pt), _tr_y->value(t, pt));

  case 3:
    return Point(_tr_x->value(t, pt), _tr_y->value(t, pt), _tr_z->value(t, pt));

  default:
    mooseError("Unsupported dimension");
  }

  return pt;
}
