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

  ElemInfo(const ElemInfo & elem_info, unsigned int /*side*/)
    : _elem(nullptr), _volume(elem_info.volume()), _subdomain_id(Moose::INVALID_BLOCK_ID)
  {
  }

  const Elem * elem() const { return _elem; }
  const Real & volume() const { return _volume; }
  const Point & centroid() const { return _centroid; }
  const SubdomainID & subdomain_id() const { return _subdomain_id; }
  void initialize_centroid(const ElemInfo & elem_info, const Point & face_center)
  {
    _centroid = 2 * face_center - elem_info.centroid();
  }

protected:
  const Elem * const _elem;
  const Real _volume;
  Point _centroid;
  const SubdomainID _subdomain_id;
};
