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

#include <map>
#include <set>

class MooseMesh;
namespace libMesh
{
class Elem;
class Node;
}

/// This data structure is used to store geometric and variable related
/// metadata about each cell face in the mesh.  This info is used by face loops
/// (e.g. for finite volumes method numerical flux loop).  These objects can be
/// created and cached up front.  Since it only stores information that changes
/// when the mesh is modified it only needs an update whenever the mesh
/// changes.
class FaceInfo
{
public:
  FaceInfo(const Elem * elem, unsigned int side, const Elem * neighbor);

  /// This enum is used to indicate which side(s) of a face a particular
  /// variable is defined on.  This is important for certain BC-related finite
  /// volume calculations. Because of the way side-sets and variable
  /// block-restriction work in MOOSE, there may be boundary conditions applied
  /// to internal faces on the mesh where a variable is only active on one or
  /// even zero sides of the face.  For such faces, FV needs to know which
  /// sides (if any) to add BC residual contributions to.
  enum class VarFaceNeighbors
  {
    BOTH,
    NEITHER,
    ELEM,
    NEIGHBOR
  };

  /// Returns the face area of face id
  Real faceArea() const { return _face_area; }

  /// Sets/gets the coordinate transformation factor (for e.g. rz, spherical
  /// coords) to be used for integration over faces.
  Real & faceCoord() { return _face_coord; }
  Real faceCoord() const { return _face_coord; }

  /// Returns the unit normal vector for the face oriented outward from the face's elem element.
  const Point & normal() const { return _normal; }

  /// Returns true if this face resides on the mesh boundary.
  bool isBoundary() const { return (_neighbor == nullptr); }

  /// Returns the coordinates of the face centroid.
  const Point & faceCentroid() const { return _face_centroid; }

  ///@{
  /// Returns the elem and neighbor elements adjacent to the face.
  /// If a face is on a mesh boundary, the neighborPtr
  /// will return nullptr - the elem will never be null.
  const Elem & elem() const { return *_elem; }
  const Elem * neighborPtr() const { return _neighbor; }
  const Elem & neighbor() const
  {
    if (!_neighbor)
      mooseError("FaceInfo object 'const Elem & neighbor()' is called but neighbor element pointer "
                 "is null. This occurs for faces at the domain boundary");
    return *_neighbor;
  }
  ///@}

  /// Returns the element centroids of the elements on the elem and neighbor sides of the face.
  /// If no neighbor face is defined, a "ghost" neighbor centroid is calculated by
  /// reflecting/extrapolating from the elem centroid through the face centroid
  /// - i.e. the vector from the elem element centroid to the face centroid is
  /// doubled in length.  The tip of this new vector is the neighbor centroid.
  /// This is important for FV dirichlet BCs.
  const Point & elemCentroid() const { return _elem_centroid; }
  const Point & neighborCentroid() const { return _neighbor_centroid; }
  ///@}

  ///@{
  /// Returns the elem and neighbor centroids. If no neighbor element exists, then
  /// the maximum unsigned int is returned for the neighbor side ID.
  unsigned int elemSideID() const { return _elem_side_id; }
  unsigned int neighborSideID() const { return _neighbor_side_id; }
  ///@}

  ///@{
  /// This is just a convenient cache of DOF indices (into the solution
  /// vector) associated with each variable on this face.
  const std::vector<dof_id_type> & elemDofIndices(const std::string & var_name) const
  {
    auto it = _elem_dof_indices.find(var_name);
    if (it == _elem_dof_indices.end())
      mooseError("Variable ", var_name, " not found in FaceInfo object");
    return it->second;
  }
  std::vector<dof_id_type> & elemDofIndices(const std::string & var_name)
  {
    return _elem_dof_indices[var_name];
  }
  const std::vector<dof_id_type> & neighborDofIndices(const std::string & var_name) const
  {
    auto it = _neighbor_dof_indices.find(var_name);
    if (it == _neighbor_dof_indices.end())
      mooseError("Variable ", var_name, " not found in FaceInfo object");
    return it->second;
  }
  std::vector<dof_id_type> & neighborDofIndices(const std::string & var_name)
  {
    return _neighbor_dof_indices[var_name];
  }
  ///@}

  /// Returns which side(s) the given variable is defined on for this face.
  VarFaceNeighbors faceType(const std::string & var_name) const
  {
    auto it = _face_types_by_var.find(var_name);
    if (it == _face_types_by_var.end())
      mooseError("Variable ", var_name, " not found in variable to VarFaceNeighbors map");
    return it->second;
  }
  /// Mutably returns which side(s) the given variable is defined on for this face.
  VarFaceNeighbors & faceType(const std::string & var_name) { return _face_types_by_var[var_name]; }
  const std::set<BoundaryID> & boundaryIDs() const { return _boundary_ids; }

  /// Returns the set of boundary ids for all boundaries that include this face.
  std::set<BoundaryID> & boundaryIDs() { return _boundary_ids; }

  /// Return the element volume
  Real elemVolume() const { return _elem_volume; }

  /// Return the neighbor volume
  Real neighborVolume() const { return _neighbor_volume; }

  /// Return the geometric weighting factor
  Real gC() const { return _gc; }

  const RealVectorValue & dCF() const { return _d_cf; }

  Real dCFMag() const { return _d_cf_mag; }

  const RealVectorValue & eCF() const { return _e_cf; }

  processor_id_type processor_id() const { return _processor_id; }

  const std::vector<const Node *> & vertices() const { return _vertices; }

private:
  Real _face_area;
  Real _face_coord = 0;
  Real _elem_volume;
  Real _neighbor_volume;
  Point _normal;

  /// the elem and neighbor elems
  const Elem * _elem;
  const Elem * _neighbor;

  /// the elem and neighbor local side ids
  unsigned int _elem_side_id;
  unsigned int _neighbor_side_id;

  Point _elem_centroid;
  Point _neighbor_centroid;
  Point _face_centroid;

  /// Geometric weighting factor
  Real _gc;

  /// cached locations of variables in solution vectors
  /// TODO: make this more efficient by not using a map if possible
  std::map<std::string, std::vector<dof_id_type>> _elem_dof_indices;
  std::map<std::string, std::vector<dof_id_type>> _neighbor_dof_indices;

  /// a map that provides the information what face type this is for each variable
  std::map<std::string, VarFaceNeighbors> _face_types_by_var;

  /// the set of boundary ids that this face is associated with
  std::set<BoundaryID> _boundary_ids;

  const processor_id_type _processor_id;

  std::vector<const Node *> _vertices;

  RealVectorValue _d_cf;
  Real _d_cf_mag;
  RealVectorValue _e_cf;
};
