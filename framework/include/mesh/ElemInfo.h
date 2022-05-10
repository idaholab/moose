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
    : _elem(elem),
      _volume(_elem->volume()),
      _centroid(_elem->vertex_average()),
      _subdomain_id(elem->subdomain_id())
  {
  }

  /// Construct using the volume of the element owning the face. This constructor
  /// is used for creating ElemInfo for fake elements for cases when the
  /// elem pointer would be invalid
  ElemInfo(Real volume) : _elem(nullptr), _volume(volume), _subdomain_id(Moose::INVALID_BLOCK_ID) {}

  const Elem * elem() const { return _elem; }
  Real volume() const { return _volume; }
  const Point & centroid() const { return _centroid; }
  SubdomainID subdomain_id() const { return _subdomain_id; }

  /// This is used to initialize the centroids of fake elements
  void initialize_centroid(const ElemInfo & elem_info, const Point & face_center)
  {
    if (_elem)
      mooseError("The centroid has already been initialized!");
    _centroid = 2 * face_center - elem_info.centroid();
  }

protected:
  /// Reference to the element in libmesh
  const Elem * const _elem;
  /// Volume of the element
  const Real _volume;
  /// Centroid of the element
  Point _centroid;
  /// Subdomain ID of the element
  const SubdomainID _subdomain_id;
};
