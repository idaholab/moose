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

ElementPairInfo::ElementPairInfo(const Elem * elem1,
                                 const Elem * elem2,
                                 const std::vector<Point> & elem1_constraint_q_point,
                                 const std::vector<Point> & elem2_constraint_q_point,
                                 const std::vector<Real> & elem1_constraint_JxW,
                                 const std::vector<Real> & elem2_constraint_JxW,
                                 const Point & elem1_normal,
                                 const Point & elem2_normal)
  : _elem1(elem1),
    _elem2(elem2),
    _elem1_constraint_q_point(elem1_constraint_q_point),
    _elem2_constraint_q_point(elem2_constraint_q_point),
    _elem1_constraint_JxW(elem1_constraint_JxW),
    _elem2_constraint_JxW(elem2_constraint_JxW),
    _elem1_normal(elem1_normal),
    _elem2_normal(elem2_normal)
{
}

ElementPairInfo::~ElementPairInfo() {}

void
ElementPairInfo::update(const std::vector<Point> & elem1_constraint_q_point,
                        const std::vector<Point> & elem2_constraint_q_point,
                        const std::vector<Real> & elem1_constraint_JxW,
                        const std::vector<Real> & elem2_constraint_JxW,
                        const Point & elem1_normal,
                        const Point & elem2_normal)
{
  _elem1_constraint_q_point = elem1_constraint_q_point;
  _elem2_constraint_q_point = elem2_constraint_q_point;
  _elem1_constraint_JxW = elem1_constraint_JxW;
  _elem2_constraint_JxW = elem2_constraint_JxW;
  _elem1_normal = elem1_normal;
  _elem2_normal = elem2_normal;
}
