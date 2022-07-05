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
  FaceInfo(const ElemInfo * elem_info, unsigned int side);

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
  bool isBoundary() const { return (_elem_info->elem()->neighbor_ptr(_elem_side_id) == nullptr); }

  bool facesRemote() const
  {
    return (_elem_info->elem()->neighbor_ptr(_elem_side_id) == remote_elem);
  }

  /// Returns the coordinates of the face centroid.
  const Point & faceCentroid() const { return _face_centroid; }

  ///@{
  /// Returns the skewness-correction vector (vector between the approximate and real face
  /// centroids).
  const Point skewnessCorrectionVector() const
  {
    if (!_neighbor_info)
      mooseError("The neighbor info does not exist so the double-sided skewness-correction vector "
                 "cannot be returned!");

    Point r_intersection =
        _elem_info->centroid() +
        (((_face_centroid - _elem_info->centroid()) * _normal) / (_e_cn * _normal)) * _e_cn;

    return _face_centroid - r_intersection;
  }

  const Point singleSidedSkewnessCorrectionVector(bool elem_side = true) const
  {
    if (!_neighbor_info && !elem_side)
      mooseError("The neighbor info does not exist so the single-sided skewness-correction vector "
                 "cannot be returned from the neighbor side!");
    if (elem_side)
      return _face_centroid - _r_cf;
    else
      return _face_centroid - _r_nf;
  }
  ///@}

  ///@{
  /// Returns the elem and neighbor elements adjacent to the face.
  /// If a face is on a mesh boundary, the neighborPtr
  /// will return nullptr - the elem will never be null.
  const Elem & elem() const { return *(_elem_info->elem()); }
  const Elem * elemPtr() const { return _elem_info->elem(); }
  const Elem & neighbor() const
  {
    if (!_neighbor_info)
      mooseError("FaceInfo object 'const Elem & neighbor()' is called but neighbor element pointer "
                 "is null. This occurs for faces at the domain boundary");
    return *(_neighbor_info->elem());
  }
  const Elem * neighborPtr() const
  {
    if (!_neighbor_info)
      return nullptr;
    return _neighbor_info->elem();
  }
  ///@}

  /// Returns the element centroids of the elements on the elem and neighbor sides of the face.
  /// If no neighbor face is defined, a "ghost" neighbor centroid is calculated by
  /// reflecting/extrapolating from the elem centroid through the face centroid
  /// - i.e. the vector from the elem element centroid to the face centroid is
  /// doubled in length.  The tip of this new vector is the neighbor centroid.
  /// This is important for FV dirichlet BCs.
  const Point & elemCentroid() const { return _elem_info->centroid(); }
  const Point & neighborCentroid() const
  {
    if (!_neighbor_info)
      mooseError("You are requesting the centroid of an invalid neighbor!");
    return _neighbor_info->centroid();
  }
  ///@}

  ///@{
  /// Returns the elem and neighbor subdomain IDs. If no neighbor element exists, then
  /// an invalid ID is returned for the neighbor subdomain ID.
  SubdomainID elemSubdomainID() const { return _elem_info->subdomain_id(); }
  SubdomainID neighborSubdomainID() const
  {
    if (!_neighbor_info)
      return Elem::invalid_subdomain_id;
    return _neighbor_info->subdomain_id();
  }
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

  bool varDefinedOnElem(const std::string & var_name) const
  {
    auto it = _face_types_by_var.find(var_name);
    if (it == _face_types_by_var.end())
      mooseError("Variable ", var_name, " not found in variable to VarFaceNeighbors map");

    const VarFaceNeighbors & ft = faceType(var_name);
    return (ft == FaceInfo::VarFaceNeighbors::BOTH || ft == FaceInfo::VarFaceNeighbors::ELEM);
  }

  bool varDefinedOnNeighbor(const std::string & var_name) const
  {
    auto it = _face_types_by_var.find(var_name);
    if (it == _face_types_by_var.end())
      mooseError("Variable ", var_name, " not found in variable to VarFaceNeighbors map");

    const VarFaceNeighbors & ft = faceType(var_name);
    return (ft == FaceInfo::VarFaceNeighbors::BOTH || ft == FaceInfo::VarFaceNeighbors::ELEM);
  }

  const std::set<BoundaryID> & boundaryIDs() const { return _boundary_ids; }

  /// Returns the set of boundary ids for all boundaries that include this face.
  std::set<BoundaryID> & boundaryIDs() { return _boundary_ids; }

  /// Return the element volume
  Real elemVolume() const { return _elem_info->volume(); }

  /// Return the neighbor volume
  Real neighborVolume() const
  {
    if (!_neighbor_info)
      mooseError("You are requesting the volume of an invalid neighbor!");
    return _neighbor_info->volume();
  }

  /// Return the geometric weighting factor
  Real gC() const { return _gc; }

  /**
   * @return the distance vector drawn from centroid C to F, or in terms of MOOSE implementation,
   * the distance vector obtained from subtracting the element centroid from the neighbor centroid
   */
  const Point & dCN() const
  {
    if (!_neighbor_info)
      mooseError(
          "The neighbor info does not exist so the cell-neighbor vector cannot be returned!");
    return _d_cn;
  }

  /**
   * @return the magnitude of the distance vector between centroids C and F, or in terms of MOOSE
   * implementation, the magnitude of the distance vector between neighbor and element centroids
   */
  Real dCNMag() const
  {
    if (!_neighbor_info)
      mooseError(
          "The neighbor info does not exist so the cell-neighbor distance cannot be returned!");
    return _d_cn_mag;
  }

  Real cellCenterToFaceDistance(bool elem_side = true) const
  {
    if (!_neighbor_info && !elem_side)
      mooseError("The neighbor info does not exist so the distance to the face cannot be returned "
                 "from the neighbor side!");
    if (elem_side)
      return _r_cf_mag;
    else
      return _r_nf_mag;
  }

  const Point & cellCenterToFaceVector(bool elem_side = true) const
  {
    if (!_neighbor_info && !elem_side)
      mooseError("The neighbor info does not exist so the vector to the face cannot be returned "
                 "from the neighbor side!");
    if (elem_side)
      return _r_cf;
    else
      return _r_nf;
  }

  /**
   * @return the normalized (e.g. unit) distance vector drawn from centroid C to F, or in terms of
   * MOOSE implementation, the normalized (e.g. unit) distance vector obtained from subtracting the
   * element centroid from the neighbor centroid
   */
  const Point & eCN() const { return _e_cn; }

  /**
   * @return the ID of the processor that owns this object
   */
  processor_id_type processor_id() const { return _processor_id; }

  /**
   * @return a unique identifier of this face object. It's formed using the element id and the
   * element's side that corresponds to this face
   */
  const std::pair<dof_id_type, unsigned int> & id() const { return _id; }

  /**
   * Takes the ElemInfo of the neighbor cell and computes interpolation weights
   * together with other quantities used to generate spatial operators.
   */
  void computeCoefficients(const ElemInfo * const neighbor_info);

private:
  /// the elem and neighbor elems
  const ElemInfo * _elem_info;
  const ElemInfo * _neighbor_info;

  Real _face_coord = 0;
  Point _normal;

  const processor_id_type _processor_id;
  const std::pair<dof_id_type, unsigned int> _id;

  /// the elem local side id
  const unsigned int _elem_side_id;
  unsigned int _neighbor_side_id;

  Real _face_area;
  Point _face_centroid;

  /// the distance vector between neighbor and element centroids
  Point _d_cn;
  Point _r_nf;
  Point _r_cf;
  Point _e_cn;

  /// the distance norm between neighbor and element centroids
  Real _d_cn_mag;
  Real _r_nf_mag;
  Real _r_cf_mag;

  /// Geometric weighting factor for face value interpolation
  Real _gc;

  /// a map that provides the information what face type this is for each variable
  std::map<std::string, VarFaceNeighbors> _face_types_by_var;

  /// the set of boundary ids that this face is associated with
  std::set<BoundaryID> _boundary_ids;
};
