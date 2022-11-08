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
#include "libmesh/elem.h"

#include <map>
#include <set>
#include <memory>

class FaceInfo;

/// Class used for caching additional information for elements
/// such as the volume and centroid. This also supports the
/// ghost elements used in the finite volume setting (for the time being).
class ElemInfo
{
public:
  /// Constructor using a real element from libmesh
  ElemInfo(const Elem * const elem);

  /// Default constructor
  ElemInfo() : _elem(nullptr) {}

  const Elem * elem() const { return _elem; }
  Real volume() const { return _volume; }
  const Point & centroid() const { return _centroid; }

  /// We return the subdomain ID of the corresponding libmesh element.
  SubdomainID subdomain_id() const { return _elem->subdomain_id(); }

protected:
  /// Reference to the element in libmesh
  const Elem * const _elem;
  /// Volume of the element
  Real _volume;
  /// Centroid of the element
  Point _centroid;
};
