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
  Real coordFactor() const { return _coord_transform_factor; }
  Real & coordFactor() { return _coord_transform_factor; }
  const std::vector<std::vector<dof_id_type>> & dofIndices() const { return _dof_indices; }
  std::vector<std::vector<dof_id_type>> & dofIndices() { return _dof_indices; }

  /// We return the subdomain ID of the corresponding libmesh element.
  SubdomainID subdomain_id() const { return _elem->subdomain_id(); }

protected:
  /// Reference to the element in libmesh
  const Elem * const _elem;
  /// Volume of the element
  Real _volume;
  /// Centroid of the element
  Point _centroid;
  /// Cached coordinate transformation factor
  Real _coord_transform_factor;
  /// Cached dof indices mainly for segregated linear FV evaluations
  /// with the following structure: _dof_indices[system_number][variable_number] = dof_index
  /// Systems with no FV variables will store an empty vector and should not be accessed.
  /// This will be checked through multiple asserts in the assembly routines.
  /// Furthermore, if the current variable is not active on the subdomain or if it
  /// is an FE variable of this element, we return an invalid_dof_index.
  std::vector<std::vector<dof_id_type>> _dof_indices;
};
