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

#include "ElementPairInfo.h"

ElementPairInfo::ElementPairInfo(const Elem * elem,
                                 const std::vector<Point> & constraint_q_point,
                                 const std::vector<Real> & constraint_JxW,
                                 const Point & normal)
  : _elem(elem),
    _constraint_q_point(constraint_q_point),
    _constraint_JxW(constraint_JxW),
    _normal(normal)
{}

ElementPairInfo::~ElementPairInfo()
{
  _constraint_q_point.clear();
  _constraint_JxW.clear();
}
