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

#ifndef MAXQPSTHREAD_H
#define MAXQPSTHREAD_H

#include "ParallelUniqueId.h"

// libMesh includes
#include "libmesh/elem_range.h"
#include "libmesh/enum_order.h"
#include "libmesh/enum_quadrature_type.h"

// Forward declarations
class FEProblemBase;

/**
 * This class determines the maximum number of Quadrature Points and
 * Shape Functions used for a given simulation based on the variable
 * discretizations, and quadrature rules used for all variables in the
 * system.
 */
class MaxQpsThread
{
public:
  MaxQpsThread(FEProblemBase & fe_problem, QuadratureType type, Order order, Order face_order);

  // Splitting Constructor
  MaxQpsThread(MaxQpsThread & x, Threads::split split);

  void operator()(const ConstElemRange & range);

  void join(const MaxQpsThread & y);

  unsigned int max() const { return _max; }

  unsigned int max_shape_funcs() const { return _max_shape_funcs; }

protected:
  FEProblemBase & _fe_problem;

  QuadratureType _qtype;
  Order _order;
  Order _face_order;

  THREAD_ID _tid;

  /// Maximum number of qps encountered
  unsigned int _max;

  /// Maximum number of shape functions encountered
  unsigned int _max_shape_funcs;
};

#endif // MAXQPSTHREAD_H
