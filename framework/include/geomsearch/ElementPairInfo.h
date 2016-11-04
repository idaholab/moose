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

#ifndef ELEMENTPAIRINFO_H
#define ELEMENTPAIRINFO_H

// MOOSE includes
#include "Moose.h"

// libmesh includes
#include "libmesh/vector_value.h"
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

#endif // ELEMENTPAIRLOCATOR_H
