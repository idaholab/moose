//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"

#include "libmesh/point.h"

// libMesh forward declarations
namespace libMesh
{
class Node;
class Elem;
}

/**
 * This is the ElementPairInfo class.  This is a base class that
 * stores information used for integration in element to element constraints.
 */

class ElementPairInfo
{
public:
  ElementPairInfo(const Elem * elem1,
                  const Elem * elem2,
                  const std::vector<Point> & elem1_constraint_q_point,
                  const std::vector<Point> & elem2_constraint_q_point,
                  const std::vector<Real> & elem1_constraint_JxW,
                  const std::vector<Real> & elem2_constraint_JxW,
                  const Point & elem1_normal,
                  const Point & elem2_normal);

  virtual ~ElementPairInfo();

  virtual void update(const std::vector<Point> & elem1_constraint_q_point,
                      const std::vector<Point> & elem2_constraint_q_point,
                      const std::vector<Real> & elem1_constraint_JxW,
                      const std::vector<Real> & elem2_constraint_JxW,
                      const Point & elem1_normal,
                      const Point & elem2_normal);

  const Elem * _elem1;
  const Elem * _elem2;
  std::vector<Point> _elem1_constraint_q_point;
  std::vector<Point> _elem2_constraint_q_point;
  std::vector<Real> _elem1_constraint_JxW;
  std::vector<Real> _elem2_constraint_JxW;
  Point _elem1_normal;
  Point _elem2_normal;
};
