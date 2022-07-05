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

/// Type containing additional information (above the libmesh data) for elements
/// suck as the volume and centroid
class ElemInfo
{
public:
  /// Constructor using a real element
  ElemInfo(const Elem * const elem)
    : _elem(elem), _volume(_elem->volume()), _centroid(_elem->vertex_average())
  {
  }

  const Elem * elem() const { return _elem; }
  Real volume() const { return _volume; }
  const Point & centroid() const { return _centroid; }
  SubdomainID subdomain_id() const { return _elem->subdomain_id(); }

protected:
  /// Reference to the element in libmesh
  const Elem * const _elem;
  /// Volume of the element
  const Real _volume;
  /// Centroid of the element
  Point _centroid;
};
