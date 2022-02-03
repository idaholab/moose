//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <list>

// MOOSE includes
#include "Moose.h"
#include "DataIO.h"
#include "ElementPairInfo.h"

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
  ElementPairLocator(unsigned int interface_id) : _elem_pairs(NULL)
  {
    _interface_id = interface_id;
  }

  virtual ~ElementPairLocator() {}

  typedef std::list<std::pair<const Elem *, const Elem *>> ElementPairList;

  virtual void reinit(){};

  virtual void update(){};

  const ElementPairList & getElemPairs() const
  {
    if (_elem_pairs == NULL)
      mooseError("_elem_pairs has not yet been initialized and it needs to be initialized by a "
                 "derived class");
    return *_elem_pairs;
  }

  const ElementPairInfo & getElemPairInfo(std::pair<const Elem *, const Elem *> elem_pair) const
  {
    std::map<std::pair<const Elem *, const Elem *>, ElementPairInfo>::const_iterator it =
        _element_pair_info.find(elem_pair);
    if (it == _element_pair_info.end())
      mooseError("Could not find ElemenPairInfo for specified element pair");
    return it->second;
  }

protected:
  const ElementPairList * _elem_pairs;
  std::map<std::pair<const Elem *, const Elem *>, ElementPairInfo> _element_pair_info;
  unsigned int _interface_id;
};
