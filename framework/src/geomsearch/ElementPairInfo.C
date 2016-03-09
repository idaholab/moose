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

ElementPairInfo::ElementPairInfo(const Elem * elem, const std::vector<Point> & q_point, const std::vector<Real> & JxW, const Point & normal)
  : _elem(elem),
    _q_point(q_point),
    _JxW(JxW),
    _normal(normal)
{}

ElementPairInfo::~ElementPairInfo()
{
  _q_point.clear();
  _JxW.clear();
}
