//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
