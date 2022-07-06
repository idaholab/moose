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

  /// Construct an empty shell for a ghost element
  ElemInfo();

  /// Initialize the ghost element using a real ElemInfo and a FaceInfo
  void initialize(const ElemInfo & ei, const FaceInfo & fi);

  /// Check if the corresponding element is a ghost
  bool isGhost() const { return _elem == nullptr; }

  const Elem * elem() const { return _elem; }
  Real volume() const { return _volume; }
  const Point & centroid() const { return _centroid; }

  /// We return the subdomain ID of the corresponding libmesh element. If we
  /// are on a ghost cell, this ID is invalid.
  SubdomainID subdomain_id() const
  {
    if (isGhost())
      return Moose::INVALID_BLOCK_ID;
    else
      return _elem->subdomain_id();
  }

protected:
  /// Reference to the element in libmesh
  const Elem * const _elem;
  /// Volume of the element
  Real _volume;
  /// Centroid of the element
  Point _centroid;
};
