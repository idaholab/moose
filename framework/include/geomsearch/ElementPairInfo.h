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
#include "DataIO.h"

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

  ElementPairInfo(const Elem * elem,
                  const std::vector<Point> & constraint_q_point,
                  const std::vector<Real> & constraint_JxW,
                  const Point & normal);

  virtual ~ElementPairInfo();

public:
  const Elem * _elem;
  std::vector<Point> _constraint_q_point;
  std::vector<Real> _constraint_JxW;
  Point _normal;
};

#endif // ELEMENTPAIRLOCATOR_H
