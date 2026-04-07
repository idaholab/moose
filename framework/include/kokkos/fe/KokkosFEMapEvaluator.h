//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"
#include "libmesh/kokkos/fe_types.h"
#include "libmesh/kokkos/fe_lagrange_1d.h"
#include "libmesh/kokkos/fe_lagrange_2d.h"
#include "libmesh/kokkos/fe_lagrange_3d.h"

namespace Moose::Kokkos
{
using namespace libMesh::Kokkos;

/**
 * CPU-side evaluator for geometric map arrays.
 *
 * Provides static helpers that fill the four geometric-map Array2D objects in
 * KokkosAssembly::initShape() without calling libMesh FEBase::reinit():
 *
 *   _map_phi            parent LAGRANGE shape functions at volume quad points
 *   _map_grad_phi       their reference-domain gradients
 *   _map_psi_face       side-element LAGRANGE shape functions at face quad points
 *   _map_grad_psi_face  their reference-domain gradients (dpsidxi, dpsideta)
 *
 * Each function both creates and fills its destination array.  All four must be
 * called from CPU code only (they are compiled only when MOOSE_KOKKOS_SCOPE is
 * defined, i.e. from .K translation units).
 *
 * Coordinate conventions match libMesh:
 *   - qpts  are in the parent element's reference coordinate system
 *   - face_qpts  are in the side element's reference coordinate system
 *     (as stored in _q_points_face, which is initialised by qrule_face->init(*side_elem))
 */
class FEMapEvaluator
{
public:
#ifdef MOOSE_KOKKOS_SCOPE

  /**
   * Fill dest = _map_phi(sid, etid) for parent topology @p topo.
   *
   * dest(node, qp) = phi_node(qpts[qp])
   *
   * @param dest   Array2D<Real> to create and fill (node × qp)
   * @param topo   Parent element topology
   * @param qpts   Volume quad points in the parent reference coordinate system
   */
  static inline void
  fillMapPhi(Array2D<Real> & dest, FEElemTopology topo, const Array<Real3> & qpts)
  {
    const unsigned int n_qp = qpts.size();
    switch (topo)
    {
      case FEElemTopology::EDGE2:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge2Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Edge2Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::EDGE3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Edge3Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TRI3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Tri3Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TRI6:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri6Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Tri6Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD4:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad4Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Quad4Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD8:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad8Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Quad8Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD9:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad9Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Quad9Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TET4:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tet4Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Tet4Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TET10:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tet10Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Tet10Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::HEX8:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Hex8Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Hex8Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::HEX20:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Hex20Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Hex20Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::HEX27:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Hex27Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) =
                FEEvaluator<LagrangeTag, Hex27Tag>::shape(i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      default:
        mooseError("FEMapEvaluator::fillMapPhi: unsupported element topology");
    }
  }

  /**
   * Fill dest = _map_grad_phi(sid, etid) for parent topology @p topo.
   *
   * dest(node, qp) = grad_phi_node(qpts[qp])  (reference-domain gradient)
   *
   * @param dest   Array2D<Real3> to create and fill (node × qp)
   * @param topo   Parent element topology
   * @param qpts   Volume quad points in the parent reference coordinate system
   */
  static inline void
  fillMapGradPhi(Array2D<Real3> & dest, FEElemTopology topo, const Array<Real3> & qpts)
  {
    const unsigned int n_qp = qpts.size();
    switch (topo)
    {
      case FEElemTopology::EDGE2:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge2Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Edge2Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::EDGE3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Edge3Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TRI3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Tri3Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TRI6:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri6Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Tri6Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD4:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad4Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Quad4Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD8:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad8Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Quad8Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD9:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad9Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Quad9Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TET4:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tet4Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Tet4Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::TET10:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tet10Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Tet10Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::HEX8:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Hex8Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Hex8Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::HEX20:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Hex20Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Hex20Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      case FEElemTopology::HEX27:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Hex27Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Hex27Tag>::grad_shape(
                i, qpts[qp](0), qpts[qp](1), qpts[qp](2));
        break;
      }
      default:
        mooseError("FEMapEvaluator::fillMapGradPhi: unsupported element topology");
    }
  }

  /**
   * Fill dest = _map_psi_face(sid, etid)(side) for side topology @p side_topo.
   *
   * dest(node, qp) = psi_node(face_qpts[qp])
   *
   * face_qpts are the face quadrature points expressed in the side element's own
   * reference coordinate system (as stored in _q_points_face, which is populated
   * by qrule_face->init(*elem->side_ptr(side))).
   *
   * @param dest       Array2D<Real> to create and fill (side_node × face_qp)
   * @param side_topo  Side element topology (from getSideTopology(parent_topo))
   * @param face_qpts  Face quad points in the side element's reference coordinate system
   */
  static inline void
  fillMapPsiFace(Array2D<Real> & dest,
                 FEElemTopology side_topo,
                 const Array<Real3> & face_qpts)
  {
    const unsigned int n_qp = face_qpts.size();
    switch (side_topo)
    {
      case FEElemTopology::EDGE2:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge2Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Edge2Tag>::shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
        break;
      }
      case FEElemTopology::EDGE3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Edge3Tag>::shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
        break;
      }
      case FEElemTopology::TRI3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Tri3Tag>::shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
        break;
      }
      case FEElemTopology::TRI6:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri6Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Tri6Tag>::shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD4:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad4Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Quad4Tag>::shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD8:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad8Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Quad8Tag>::shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
        break;
      }
      case FEElemTopology::QUAD9:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad9Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
            dest(i, qp) = FEEvaluator<LagrangeTag, Quad9Tag>::shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
        break;
      }
      default:
        mooseError("FEMapEvaluator::fillMapPsiFace: unsupported side topology");
    }
  }

  /**
   * Fill dest = _map_grad_psi_face(sid, etid)(side) for side topology @p side_topo.
   *
   * Matches what libMesh stores in get_dpsidxi / get_dpsideta:
   *   dest(node, qp)(0) = dpsidxi   (set when parent_dim > 1)
   *   dest(node, qp)(1) = dpsideta  (set when parent_dim > 2)
   *   dest(node, qp)(2) = 0         (never used)
   *
   * face_qpts are in the side element's reference coordinate system.
   *
   * @param dest        Array2D<Real3> to create and fill (side_node × face_qp)
   * @param side_topo   Side element topology (from getSideTopology(parent_topo))
   * @param face_qpts   Face quad points in the side element's reference coordinate system
   * @param parent_dim  Spatial dimension of the parent element
   */
  static inline void
  fillMapGradPsiFace(Array2D<Real3> & dest,
                     FEElemTopology side_topo,
                     const Array<Real3> & face_qpts,
                     unsigned int parent_dim)
  {
    const unsigned int n_qp = face_qpts.size();
    switch (side_topo)
    {
      case FEElemTopology::EDGE2:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge2Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
          {
            auto g = FEEvaluator<LagrangeTag, Edge2Tag>::grad_shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
            if (parent_dim > 1)
              dest(i, qp)(0) = g(0);
            if (parent_dim > 2)
              dest(i, qp)(1) = g(1);
          }
        break;
      }
      case FEElemTopology::EDGE3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Edge3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
          {
            auto g = FEEvaluator<LagrangeTag, Edge3Tag>::grad_shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
            if (parent_dim > 1)
              dest(i, qp)(0) = g(0);
            if (parent_dim > 2)
              dest(i, qp)(1) = g(1);
          }
        break;
      }
      case FEElemTopology::TRI3:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri3Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
          {
            auto g = FEEvaluator<LagrangeTag, Tri3Tag>::grad_shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
            if (parent_dim > 1)
              dest(i, qp)(0) = g(0);
            if (parent_dim > 2)
              dest(i, qp)(1) = g(1);
          }
        break;
      }
      case FEElemTopology::TRI6:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Tri6Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
          {
            auto g = FEEvaluator<LagrangeTag, Tri6Tag>::grad_shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
            if (parent_dim > 1)
              dest(i, qp)(0) = g(0);
            if (parent_dim > 2)
              dest(i, qp)(1) = g(1);
          }
        break;
      }
      case FEElemTopology::QUAD4:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad4Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
          {
            auto g = FEEvaluator<LagrangeTag, Quad4Tag>::grad_shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
            if (parent_dim > 1)
              dest(i, qp)(0) = g(0);
            if (parent_dim > 2)
              dest(i, qp)(1) = g(1);
          }
        break;
      }
      case FEElemTopology::QUAD8:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad8Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
          {
            auto g = FEEvaluator<LagrangeTag, Quad8Tag>::grad_shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
            if (parent_dim > 1)
              dest(i, qp)(0) = g(0);
            if (parent_dim > 2)
              dest(i, qp)(1) = g(1);
          }
        break;
      }
      case FEElemTopology::QUAD9:
      {
        constexpr unsigned int n = FEEvaluator<LagrangeTag, Quad9Tag>::n_dofs();
        dest.create(n, n_qp);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int qp = 0; qp < n_qp; ++qp)
          {
            auto g = FEEvaluator<LagrangeTag, Quad9Tag>::grad_shape(
                i, face_qpts[qp](0), face_qpts[qp](1), face_qpts[qp](2));
            if (parent_dim > 1)
              dest(i, qp)(0) = g(0);
            if (parent_dim > 2)
              dest(i, qp)(1) = g(1);
          }
        break;
      }
      default:
        mooseError("FEMapEvaluator::fillMapGradPsiFace: unsupported side topology");
    }
  }

#endif // MOOSE_KOKKOS_SCOPE
};

} // namespace Moose::Kokkos
