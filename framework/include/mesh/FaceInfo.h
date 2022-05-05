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
#include <memory>

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
  FaceInfo(const Elem * elem,
           unsigned int side,
           const Elem * neighbor,
           const std::unordered_map<Elem *, Real> & cell_volumes,
           const std::unordered_map<Elem *, Point> & cell_centroids);

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
  /// Returns the coordinates of the approximate face centroid
  /// (intersection of the face and the line between the cell centroids)
  /// in case of skewed meshes.
  const Point & rIntersection() const { return _r_intersection; }
  ///@}

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
  const Point & elemCentroid() const { return *_elem_centroid; }
  const Point & neighborCentroid() const { return *_neighbor_centroid; }
  ///@}

  ///@{
  /// Returns the elem and neighbor subdomain IDs. If no neighbor element exists, then
  /// an invalid ID is returned for the neighbor subdomain ID.
  SubdomainID elemSubdomainID() const { return _elem_subdomain_id; }
  SubdomainID neighborSubdomainID() const { return _neighbor_subdomain_id; }
  ///@}

  ///@{
  /// Returns the elem and neighbor centroids. If no neighbor element exists, then
  /// the maximum unsigned int is returned for the neighbor side ID.
  unsigned int elemSideID() const { return _elem_side_id; }
  unsigned int neighborSideID() const { return _neighbor_side_id; }
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
  Real elemVolume() const { return *_elem_volume; }

  /// Return the neighbor volume
  Real neighborVolume() const { return *_neighbor_volume; }

  /// Return the geometric weighting factor
  Real gC() const { return _gc; }

  /**
   * @return the distance vector drawn from centroid C to F, or in terms of MOOSE implementation,
   * the distance vector obtained from subtracting the element centroid from the neighbor centroid
   */
  const RealVectorValue & dCF() const { return _d_cf; }

  /**
   * @return the magnitude of the distance vector between centroids C and F, or in terms of MOOSE
   * implementation, the magnitude of the distance vector between neighbor and element centroids
   */
  Real dCFMag() const { return _d_cf_mag; }

  /**
   * @return the normalized (e.g. unit) distance vector drawn from centroid C to F, or in terms of
   * MOOSE implementation, the normalized (e.g. unit) distance vector obtained from subtracting the
   * element centroid from the neighbor centroid
   */
  const RealVectorValue & eCF() const { return _e_cf; }

  /**
   * @return the ID of the processor that owns this object
   */
  processor_id_type processor_id() const { return _processor_id; }

  /**
   * @return a unique identifier of this face object. It's formed using the element id and the
   * element's side that corresponds to this face
   */
  const std::pair<dof_id_type, unsigned int> & id() const { return _id; }

private:
  Real _face_coord = 0;
  Point _normal;

  const processor_id_type _processor_id;
  const std::pair<dof_id_type, unsigned int> _id;

  /// the elem and neighbor elems
  const Elem * const _elem;
  const Elem * const _neighbor;

  /// the elem subdomain id
  const SubdomainID _elem_subdomain_id;

  /// the elem local side id
  const unsigned int _elem_side_id;

  const Point & _elem_centroid;
  const Real _elem_volume;

  /// A unique_ptr to the face element built from \p _elem and \p _elem_side_id
  std::unique_ptr<Elem> _face;

  const Real _face_area;
  const Point _face_centroid;

  /// Whether neighbor is non-null and non-remote
  const bool _valid_neighbor;

  /// the neighbor subdoman id
  const SubdomainID _neighbor_subdomain_id;

  /// the neighbor local side ide
  const unsigned int _neighbor_side_id;

  const Point * _neighbor_centroid;
  const Real * _neighbor_volume;

  /// the distance vector between neighbor and element centroids
  const RealVectorValue _d_cf;

  /// the distance norm between neighbor and element centroids
  const Real _d_cf_mag;

  /// The unit normal vector pointing from element center C to element center F
  const RealVectorValue _e_cf;

  /// The vector to the intersection of d_{CF} and the face.
  Point _r_intersection;

  /// Geometric weighting factor for face value interpolation
  Real _gc;

  /// a map that provides the information what face type this is for each variable
  std::map<std::string, VarFaceNeighbors> _face_types_by_var;

  /// the set of boundary ids that this face is associated with
  std::set<BoundaryID> _boundary_ids;
};
