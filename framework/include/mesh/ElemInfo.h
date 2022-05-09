//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "libmesh/point.h"
#include "libmesh/vector_value.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"

#include <map>
#include <set>
#include <memory>

class MooseMesh;

class ElemInfo
{
public:
  ElemInfo(const Elem * const elem)
    : _elem(elem),
      _volume(_elem->volume()),
      _centroid(_elem->vertex_average()),
      _subdomain_id(elem->subdomain_id())
  {
  }

  ElemInfo(const ElemInfo & elem_info, unsigned int side)
    : _elem(nullptr),
      _volume(elem_info.volume()),
      _centroid(2 * (const_cast<Elem *>(elem_info.elem())->build_side_ptr(side)->vertex_average() -
                     elem_info.centroid()) +
                elem_info.centroid()),
      _subdomain_id(Moose::INVALID_BLOCK_ID)
  {
  }

  const Elem * elem() const { return _elem; }
  const Real & volume() const { return _volume; }
  const Point & centroid() const { return _centroid; }
  const SubdomainID & subdomain_id() const { return _subdomain_id; }

protected:
  const Elem * const _elem;
  const Real _volume;
  const Point _centroid;
  const SubdomainID _subdomain_id;
};
