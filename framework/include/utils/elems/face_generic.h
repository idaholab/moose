// The libMesh Finite Element Library.
// Copyright (C) 2002-2023 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef LIBMESH_FACE_GENERIC_H
#define LIBMESH_FACE_GENERIC_H

// Local includes
#include "libmesh/libmesh_common.h"
#include "libmesh/face.h"

namespace libMesh
{

/**
 * The \p Generic element is an collection of nodes in 2D. No assumptions
 * about node connectivity are made.
 *
 * \author Daniel Schwen
 * \date 2023
 * \brief The base class for all triangular element types.
 */
class GenericFace : public Face
{
public:
  /**
   * Generic 2D element, takes number of nodes and an optional
   * parent. Order and interpretation of the nodes is up to the user.
   */
  GenericFace(const unsigned int nn, Elem * p = nullptr) : Face(nn, 0, p, nullptr, nullptr)
  {
    // copy nodes
    _nodelinks_data.resize(nn);
    _nodes = _nodelinks_data.data();

    // Make sure the interior parent isn't undefined
    if (LIBMESH_DIM > 2)
      this->set_interior_parent(nullptr);
  }

  GenericFace(GenericFace &&) = delete;
  GenericFace(const GenericFace &) = delete;
  GenericFace & operator=(const GenericFace &) = delete;
  GenericFace & operator=(GenericFace &&) = delete;
  virtual ~GenericFace() = default;

  /**
   * \returns The \p Point associated with local \p Node \p i,
   * in master element rather than physical coordinates.
   */
  virtual Point master_point(const unsigned int /*i*/) const override final
  {
    libmesh_error_msg("Generic 2d elements have no master points");
  }

  /**
   * \returns 3.  All tri-derivatives are guaranteed to have at
   * least 3 nodes.
   */
  virtual unsigned int n_nodes() const override { return _nodelinks_data.size(); }

  /**
   * \returns 0.
   */
  virtual unsigned int n_sides() const override final { return 0; }

  /**
   * \returns 3.  All triangles have 3 vertices.
   */
  virtual unsigned int n_vertices() const override final
  {
    libmesh_error_msg("Generic 2d elements cannot distinguish between nodes and vertices");
    ;
  }

  /**
   * \returns 3.  All triangles have 3 edges.
   */
  virtual unsigned int n_edges() const override final
  {
    libmesh_error_msg("Generic 2d elements do not have fixed edges");
  }

  /**
   * \returns 4.
   */
  virtual unsigned int n_children() const override final
  {
    libmesh_error_msg("Generic 2d elements do not support adaptivity");
  }

  /**
   * \returns \p true if the specified child is on the
   * specified side.
   */
  virtual bool is_child_on_side(const unsigned int c, const unsigned int s) const override final
  {
    return false;
  }

  using Elem::key;

  virtual dof_id_type key(const unsigned int s) const override
  {
    libmesh_error_msg("not implemented");
  }

  virtual dof_id_type key() const override final { libmesh_error_msg("not implemented"); }

  virtual unsigned int local_side_node(unsigned int /*side*/,
                                       unsigned int /*side_node*/) const override
  {
    libmesh_error_msg("not implemented");
  }

  virtual unsigned int local_edge_node(unsigned int /*edge*/,
                                       unsigned int /*edge_node*/) const override
  {
    libmesh_error_msg("not implemented");
  }

  virtual std::unique_ptr<Elem> side_ptr(const unsigned int /*i*/) override final
  {
    libmesh_error_msg("not implemented");
  }

  virtual void side_ptr(std::unique_ptr<Elem> & /*elem*/, const unsigned int /*i*/) override final
  {
    libmesh_error_msg("not implemented");
  }

  virtual Real quality(const ElemQuality /*q*/) const override
  {
    libmesh_error_msg("not implemented");
  }

  virtual std::pair<Real, Real> qual_bounds(const ElemQuality q) const override
  {
    libmesh_error_msg("not implemented");
  }

  /**
   * Do not permute a generic element!
   */
  virtual unsigned int n_permutations() const override final { return 0; }

  virtual void orient(BoundaryInfo *) override final {}

  virtual ElemType type() const override { return INVALID_ELEM; }

  virtual unsigned int n_sub_elem() const override { return 1; }

  virtual bool is_vertex(const unsigned int) const override { return false; }

  virtual bool is_edge(const unsigned int) const override { return false; }
  virtual bool is_face(const unsigned int) const override { return false; }

  virtual bool is_node_on_side(const unsigned int /*n*/, const unsigned int /*s*/) const override
  {
    libmesh_error_msg("not implemented");
  }

  virtual std::vector<unsigned int> nodes_on_side(const unsigned int /*s*/) const override
  {
    libmesh_error_msg("not implemented");
  }

  virtual std::vector<unsigned int> nodes_on_edge(const unsigned int /*e*/) const override
  {
    libmesh_error_msg("not implemented");
  }

  virtual bool is_node_on_edge(const unsigned int /*n*/, const unsigned int /*e*/) const override
  {
    return false;
  }

  virtual bool has_affine_map() const override { return true; }

  virtual bool has_invertible_map(Real /*tol*/) const override { return true; }

  virtual bool is_linear() const override { return true; }

  virtual Order default_order() const override { return libMesh::FIRST; }

  virtual std::unique_ptr<Elem> build_side_ptr(const unsigned int /*i*/,
                                               bool /*proxy = false*/) override
  {
    libmesh_error_msg("not implemented");
  }

  /**
   * Rebuilds an EDGE2 coincident with face i.
   */
  virtual void build_side_ptr(std::unique_ptr<Elem> & /*elem*/, const unsigned int /*i*/) override
  {
    libmesh_error_msg("not implemented");
  }

  virtual void connectivity(const unsigned int /*sf*/,
                            const IOPackage /*iop*/,
                            std::vector<dof_id_type> & /*conn*/) const override
  {
  }

  virtual Point true_centroid() const override { libmesh_error_msg("not implemented"); }

  virtual Real volume() const override { libmesh_error_msg("not implemented"); }

  std::pair<Real, Real> min_and_max_angle() const { libmesh_error_msg("not implemented"); }

  virtual bool contains_point(const Point & /*p*/, Real /*tol*/) const override { return false; }

  // virtual BoundingBox loose_bounding_box() const override;

  virtual void permute(unsigned int /*perm_num*/) override final {}

  virtual void flip(BoundaryInfo *) override final {}

  ElemType side_type(const unsigned int /*s*/) const override final
  {
    libmesh_error_msg("not implemented");
  }

  /**
   * Matrix used to create the elements children.
   */
  virtual Real embedding_matrix(const unsigned int /*i*/,
                                const unsigned int /*j*/,
                                const unsigned int /*k*/) const override
  {
    libmesh_error_msg("not implemented");
  }

protected:
  /**
   * node locations
   */
  std::vector<Node *> _nodelinks_data;
};

} // namespace libMesh

#endif // LIBMESH_FACE_GENERIC_H
