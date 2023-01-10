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
#include "ElemInfo.h"

#include "libmesh/vector_value.h"
#include "libmesh/remote_elem.h"

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
  FaceInfo(const ElemInfo * const elem_info, const unsigned int side, const dof_id_type id);

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

  /// Return the ID of the face
  dof_id_type id() const { return _id; }

  /// Returns the face area of face id
  Real faceArea() const { return _face_area; }

  /// Sets/gets the coordinate transformation factor (for e.g. rz, spherical
  /// coords) to be used for integration over faces.
  Real & faceCoord() { return _face_coord; }
  Real faceCoord() const { return _face_coord; }

  /// Returns the unit normal vector for the face oriented outward from the face's elem element.
  const Point & normal() const { return _normal; }

  /// Returns the coordinates of the face centroid.
  const Point & faceCentroid() const { return _face_centroid; }

  /// Returns the skewness-correction vector (vector between the approximate and real face
  /// centroids).
  Point skewnessCorrectionVector() const;

  ///@{
  /// Returns the elem and neighbor elements adjacent to the face.
  /// If a face is on a mesh boundary, the neighborPtr
  /// will return nullptr - the elem will never be null.
  const Elem & elem() const { return *(_elem_info->elem()); }
  const Elem * elemPtr() const { return _elem_info->elem(); }
  const Elem & neighbor() const;
  const Elem * neighborPtr() const { return _neighbor_info ? _neighbor_info->elem() : nullptr; }
  ///@}

  /// Returns the element centroids of the elements on the elem and neighbor sides of the face.
  /// If no neighbor face is defined, a "ghost" neighbor centroid is calculated by
  /// reflecting/extrapolating from the elem centroid through the face centroid
  /// - i.e. the vector from the elem element centroid to the face centroid is
  /// doubled in length.  The tip of this new vector is the neighbor centroid.
  /// This is important for FV dirichlet BCs.
  const Point & elemCentroid() const { return _elem_info->centroid(); }
  const Point & neighborCentroid() const;
  ///@}

  ///@{
  /// Returns the elem and neighbor subdomain IDs. If no neighbor element exists, then
  /// an invalid ID is returned for the neighbor subdomain ID.
  SubdomainID elemSubdomainID() const { return _elem_info->subdomain_id(); }
  SubdomainID neighborSubdomainID() const;
  ///@}

  ///@{
  /// Returns the elem and neighbor centroids. If no neighbor element exists, then
  /// the maximum unsigned int is returned for the neighbor side ID.
  unsigned int elemSideID() const { return _elem_side_id; }
  unsigned int neighborSideID() const { return _neighbor_side_id; }
  ///@}

  /// Returns which side(s) the given variable is defined on for this face.
  VarFaceNeighbors faceType(const std::string & var_name) const;
  /// Mutably returns which side(s) the given variable is defined on for this face.
  VarFaceNeighbors & faceType(const std::string & var_name) { return _face_types_by_var[var_name]; }

  const std::set<BoundaryID> & boundaryIDs() const { return _boundary_ids; }

  /// Returns the set of boundary ids for all boundaries that include this face.
  std::set<BoundaryID> & boundaryIDs() { return _boundary_ids; }

  /// Return the element volume
  Real elemVolume() const { return _elem_info->volume(); }

  /// Return the neighbor volume
  Real neighborVolume() const;

  /// Return the geometric weighting factor
  Real gC() const { return _gc; }

  /**
   * @return the distance vector drawn from centroid C to N, or in terms of MOOSE implementation,
   * the distance vector obtained from subtracting the element centroid from the neighbor centroid
   */
  const Point & dCN() const { return _d_cn; }

  /**
   * @return the magnitude of the distance vector between centroids C and N, or in terms of MOOSE
   * implementation, the magnitude of the distance vector between neighbor and element centroids
   */
  Real dCNMag() const { return _d_cn_mag; }

  /**
   * @return the normalized (e.g. unit) distance vector drawn from centroid C to N, or in terms of
   * MOOSE implementation, the normalized (e.g. unit) distance vector obtained from subtracting the
   * element centroid from the neighbor centroid
   */
  const Point & eCN() const { return _e_cn; }

  /**
   * @return the ID of the processor that owns this object
   */
  processor_id_type processor_id() const { return _processor_id; }

  /**
   * Takes the ElemInfo of the neighbor cell and computes interpolation weights
   * together with other quantities used to generate spatial operators.
   */
  void computeInternalCoefficients(const ElemInfo * const neighbor_info);

  /**
   * Computes the interpolation weights and similar quantities with the assumption
   * that the face is on a boundary.
   */
  void computeBoundaryCoefficients();

private:
  /// the elem and neighbor elems
  const ElemInfo * const _elem_info;
  const ElemInfo * _neighbor_info;

  const dof_id_type _id;

  Real _face_coord = 0;
  Point _normal;

  const processor_id_type _processor_id;

  /// the elem local side id
  const unsigned int _elem_side_id;
  unsigned int _neighbor_side_id;

  Real _face_area;
  Point _face_centroid;

  /// the distance vector between neighbor and element centroids
  Point _d_cn;
  Point _e_cn;

  /// the distance norm between neighbor and element centroids
  Real _d_cn_mag;

  /// Geometric weighting factor for face value interpolation
  Real _gc;

  /// a map that provides the information what face type this is for each variable
  std::map<std::string, VarFaceNeighbors> _face_types_by_var;

  /// the set of boundary ids that this face is associated with
  std::set<BoundaryID> _boundary_ids;
};

inline const Elem &
FaceInfo::neighbor() const
{
  mooseAssert(_neighbor_info,
              "FaceInfo object 'const Elem & neighbor()' is called but neighbor element pointer "
              "is null. This occurs for faces at the domain boundary");
  return *(_neighbor_info->elem());
}

inline FaceInfo::VarFaceNeighbors
FaceInfo::faceType(const std::string & var_name) const
{
  auto it = _face_types_by_var.find(var_name);
  if (it == _face_types_by_var.end())
    mooseError("Variable ", var_name, " not found in variable to VarFaceNeighbors map");
  return it->second;
}

inline const Point &
FaceInfo::neighborCentroid() const
{
  mooseAssert(_neighbor_info,
              "The neighbor is not defined on this faceInfo! A possible explanation is that the "
              "face is a (physical/processor) boundary face.");
  return _neighbor_info->centroid();
}

inline SubdomainID
FaceInfo::neighborSubdomainID() const
{
  return _neighbor_info ? _neighbor_info->subdomain_id() : Moose::INVALID_BLOCK_ID;
}

inline Real
FaceInfo::neighborVolume() const
{
  mooseAssert(_neighbor_info,
              "The neighbor is not defined on this faceInfo! A possible explanation is that the "
              "face is a (physical/processor) boundary face.");
  return _neighbor_info->volume();
}
