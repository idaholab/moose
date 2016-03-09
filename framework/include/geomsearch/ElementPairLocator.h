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

#ifndef ELEMENTPAIRLOCATOR_H
#define ELEMENTPAIRLOCATOR_H

// MOOSE includes
#include "Moose.h"
#include "DataIO.h"
#include "ElementPairInfo.h"

// libmesh includes
#include "libmesh/vector_value.h"
#include "libmesh/point.h"

// libMesh forward declarations
namespace libMesh
{
  class Node;
  class Elem;
}

class ElementPairInfo;

/**
 * This is the ElementPairLocator class.  This is a base class that
 * finds the element pairs for ElementElementConstraint
 */

class ElementPairLocator
{
public:

  ElementPairLocator(unsigned int interface_id)
  {
    _interface_id = interface_id;
  }

  virtual ~ElementPairLocator()
  {
    _elem_pairs.clear();

    for (std::map<const Elem *, ElementPairInfo *>::iterator it = _element_pair_info.begin(); it != _element_pair_info.end(); ++it)
      delete it->second;
  }
 
  virtual void reinit(){};

public:

  std::vector<std::pair<const Elem*, const Elem*> > _elem_pairs;

  std::map<const Elem*, ElementPairInfo *> _element_pair_info;

private:
  unsigned int _interface_id;

};

#endif // ELEMENTPAIRLOCATOR_H
