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

#include <list>

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

/**
 * This is the ElementPairLocator class.  This is a base class that
 * finds the element pairs for ElementElementConstraint
 */

class ElementPairLocator
{
public:

  ElementPairLocator(unsigned int interface_id):
    _elem_pairs(NULL)
  {
    _interface_id = interface_id;
  }

  virtual ~ElementPairLocator()
  {
  }

  virtual void reinit(){};

  const std::list<std::pair<const Elem*, const Elem*> > & getElemPairs() const
  {
    if (_elem_pairs == NULL)
      mooseError("_elem_pairs has not yet been initialized");
    return *_elem_pairs;
  }

  const ElementPairInfo & getElemPairInfo(const Elem* elem) const
  {
    std::map<const Elem*, ElementPairInfo>::const_iterator it = _element_pair_info.find(elem);
    if (it == _element_pair_info.end())
      mooseError("Could not find ElemenPairInfo for specified element");
    return it->second;
  }

protected:

  const std::list<std::pair<const Elem*, const Elem*> > * _elem_pairs;

  std::map<const Elem*, ElementPairInfo> _element_pair_info;

private:
  unsigned int _interface_id;
};

#endif // ELEMENTPAIRLOCATOR_H
