//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose
#include "DenseMatrix.h"
#include "FindContactPoint.h"
#include "LineSegment.h"
#include "PenetrationInfo.h"

// libMesh
#include "libmesh/fe_base.h"
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/plane.h"
#include "libmesh/fe_interface.h"
#include "libmesh/dense_vector.h"
#include "libmesh/fe_base.h"
#include "libmesh/vector_value.h"

using namespace libMesh;

namespace Moose
{

/**
 * Finds the closest point (called the contact point) on the primary_elem on side "side" to the
 * secondary_point.
 *
 * @param p_info The penetration info object, contains primary_elem, side, various other information
 * @param fe_elem FE object for the element
 * @param fe_side FE object for the side
 * @param fe_side_type The type of fe_side, needed for inverse_map routines
 * @param start_with_centroid if true, start inverse mapping procedure from element centroid
 * @param tangential_tolerance 'tangential' tolerance for determining whether a contact point on a
 * side
 * @param secondary_point The physical space coordinates of the secondary node
 * @param contact_point_on_side whether or not the contact_point actually lies on _that_ side of the
 * element.
 * @param search_succeeded whether or not the search for the contact point succeeded. If not it
 * should likely be discarded
 */
void
findContactPoint(PenetrationInfo & p_info,
                 FEBase * fe_elem,
                 FEBase * fe_side,
                 FEType & fe_side_type,
                 const libMesh::Point & secondary_point,
                 bool start_with_centroid,
                 const Real tangential_tolerance,
                 bool & contact_point_on_side,
                 bool & search_succeeded)
{
  // Default to true and we'll switch on failures
  search_succeeded = true;

  const Elem * primary_elem = p_info._elem;

  unsigned int dim = primary_elem->dim();

  const Elem * side = p_info._side;

  const std::vector<libMesh::Point> & phys_point = fe_side->get_xyz();

  const std::vector<RealGradient> & dxyz_dxi = fe_side->get_dxyzdxi();
  const std::vector<RealGradient> & d2xyz_dxi2 = fe_side->get_d2xyzdxi2();
  const std::vector<RealGradient> & d2xyz_dxieta = fe_side->get_d2xyzdxideta();

  const std::vector<RealGradient> & dxyz_deta = fe_side->get_dxyzdeta();
  const std::vector<RealGradient> & d2xyz_deta2 = fe_side->get_d2xyzdeta2();
  const std::vector<RealGradient> & d2xyz_detaxi = fe_side->get_d2xyzdxideta();

  if (dim == 1)
  {
    const Node * nearest_node = side->node_ptr(0);
    p_info._closest_point = *nearest_node;
    p_info._closest_point_ref =
        primary_elem->master_point(primary_elem->get_node_index(nearest_node));
    std::vector<libMesh::Point> elem_points = {p_info._closest_point_ref};

    const std::vector<RealGradient> & elem_dxyz_dxi = fe_elem->get_dxyzdxi();

    fe_elem->reinit(primary_elem, &elem_points);
    fe_side->reinit(side, &elem_points);

    p_info._normal = elem_dxyz_dxi[0];
    if (nearest_node->id() == primary_elem->node_id(0))
      p_info._normal *= -1.0;
    p_info._normal /= p_info._normal.norm();

    libMesh::Point from_secondary_to_closest = p_info._closest_point - secondary_point;
    p_info._distance = from_secondary_to_closest * p_info._normal;
    libMesh::Point tangential = from_secondary_to_closest - p_info._distance * p_info._normal;
    p_info._tangential_distance = tangential.norm();
    p_info._dxyzdxi = dxyz_dxi;
    p_info._dxyzdeta = dxyz_deta;
    p_info._d2xyzdxideta = d2xyz_dxieta;
    p_info._side_phi = fe_side->get_phi();
    p_info._side_grad_phi = fe_side->get_dphi();
    contact_point_on_side = true;
    return;
  }

  libMesh::Point ref_point;

  if (start_with_centroid)
    ref_point = FEInterface::inverse_map(
        dim - 1, fe_side_type, side, side->vertex_average(), TOLERANCE, false);
  else
    ref_point = p_info._closest_point_ref;

  std::vector<libMesh::Point> points = {ref_point};
  fe_side->reinit(side, &points);
  RealGradient d = secondary_point - phys_point[0];

  Real update_size = std::numeric_limits<Real>::max();

  // Least squares
  for (unsigned int it = 0; it < 3 && update_size > TOLERANCE * 1e3; ++it)
  {
    DenseMatrix<Real> jac(dim - 1, dim - 1);
    jac(0, 0) = -(dxyz_dxi[0] * dxyz_dxi[0]);

    if (dim - 1 == 2)
    {
      jac(1, 0) = -(dxyz_dxi[0] * dxyz_deta[0]);
      jac(0, 1) = -(dxyz_deta[0] * dxyz_dxi[0]);
      jac(1, 1) = -(dxyz_deta[0] * dxyz_deta[0]);
    }

    DenseVector<Real> rhs(dim - 1);
    rhs(0) = dxyz_dxi[0] * d;

    if (dim - 1 == 2)
      rhs(1) = dxyz_deta[0] * d;

    DenseVector<Real> update(dim - 1);
    jac.lu_solve(rhs, update);

    ref_point(0) -= update(0);

    if (dim - 1 == 2)
      ref_point(1) -= update(1);

    points[0] = ref_point;
    fe_side->reinit(side, &points);
    d = secondary_point - phys_point[0];

    update_size = update.l2_norm();
  }

  update_size = std::numeric_limits<Real>::max();

  unsigned nit = 0;

  // Newton Loop
  const auto max_newton_its = 25;
  const auto tolerance_newton = 1e3 * TOLERANCE * TOLERANCE;
  for (; nit < max_newton_its && update_size > tolerance_newton; nit++)
  {
    d = secondary_point - phys_point[0];

    DenseMatrix<Real> jac(dim - 1, dim - 1);
    jac(0, 0) = (d2xyz_dxi2[0] * d) - (dxyz_dxi[0] * dxyz_dxi[0]);

    if (dim - 1 == 2)
    {
      jac(1, 0) = (d2xyz_dxieta[0] * d) - (dxyz_dxi[0] * dxyz_deta[0]);

      jac(0, 1) = (d2xyz_detaxi[0] * d) - (dxyz_deta[0] * dxyz_dxi[0]);
      jac(1, 1) = (d2xyz_deta2[0] * d) - (dxyz_deta[0] * dxyz_deta[0]);
    }

    DenseVector<Real> rhs(dim - 1);
    rhs(0) = -dxyz_dxi[0] * d;

    if (dim - 1 == 2)
      rhs(1) = -dxyz_deta[0] * d;

    DenseVector<Real> update(dim - 1);
    jac.lu_solve(rhs, update);

    // Improvised line search in case the update is too large and gets out of the element so bad
    // that we cannot reinit at the new point
    Real mult = 1;
    while (true)
    {
      try
      {
        ref_point(0) += mult * update(0);

        if (dim - 1 == 2)
          ref_point(1) += mult * update(1);

        points[0] = ref_point;
        fe_side->reinit(side, &points);
        d = secondary_point - phys_point[0];

        // we don't multiply by 'mult' because it is used for convergence
        update_size = update.l2_norm();
        break;
      }
      catch (libMesh::LogicError & e)
      {
        ref_point(0) -= mult * update(0);
        if (dim - 1 == 2)
          ref_point(1) -= mult * update(1);

        mult *= 0.5;
        if (mult < 1e-6)
        {
#ifndef NDEBUG
          mooseWarning("We could not solve for the contact point.", e.what());
#endif
          update_size = update.l2_norm();
          d = (secondary_point - phys_point[0]) * mult;
          break;
        }
      }
    }
    // We failed the line search, make sure to trigger the error
    if (mult < 1e-6)
    {
      nit = max_newton_its;
      update_size = 1;
      break;
    }
  }

  if (nit == max_newton_its && update_size > tolerance_newton)
  {
    search_succeeded = false;
#ifndef NDEBUG
    const auto initial_point =
        start_with_centroid ? side->vertex_average() : ref_point = p_info._closest_point_ref;
    Moose::err << "Warning!  Newton solve for contact point failed to converge!\nLast update "
                  "distance was: "
               << update_size << "\nInitial point guess:   " << initial_point
               << "\nLast considered point: " << phys_point[0]
               << "\nThis potential contact pair (face, point) will be discarded." << std::endl;
#endif
    return;
  }

  p_info._closest_point_ref = ref_point;
  p_info._closest_point = phys_point[0];
  p_info._distance = d.norm();

  if (dim - 1 == 2)
  {
    p_info._normal = dxyz_dxi[0].cross(dxyz_deta[0]);
    if (!MooseUtils::absoluteFuzzyEqual(p_info._normal.norm(), 0))
      p_info._normal /= p_info._normal.norm();
  }
  else
  {
    const Node * const * elem_nodes = primary_elem->get_nodes();
    const libMesh::Point in_plane_vector1 = *elem_nodes[1] - *elem_nodes[0];
    const libMesh::Point in_plane_vector2 = *elem_nodes[2] - *elem_nodes[0];

    libMesh::Point out_of_plane_normal = in_plane_vector1.cross(in_plane_vector2);
    out_of_plane_normal /= out_of_plane_normal.norm();

    p_info._normal = dxyz_dxi[0].cross(out_of_plane_normal);
    if (std::fabs(p_info._normal.norm()) > 1e-15)
      p_info._normal /= p_info._normal.norm();
  }

  // If the point has not penetrated the face, make the distance negative
  const Real dot(d * p_info._normal);
  if (dot > 0.0)
    p_info._distance = -p_info._distance;

  contact_point_on_side = FEInterface::on_reference_element(ref_point, side->type());

  p_info._tangential_distance = 0.0;

  if (!contact_point_on_side)
  {
    p_info._closest_point_on_face_ref = ref_point;
    restrictPointToFace(p_info._closest_point_on_face_ref, side, p_info._off_edge_nodes);

    points[0] = p_info._closest_point_on_face_ref;
    fe_side->reinit(side, &points);
    libMesh::Point closest_point_on_face(phys_point[0]);

    RealGradient off_face = closest_point_on_face - p_info._closest_point;
    Real tangential_distance = off_face.norm();
    p_info._tangential_distance = tangential_distance;
    if (tangential_distance <= tangential_tolerance)
    {
      contact_point_on_side = true;
    }
  }

  const std::vector<std::vector<Real>> & phi = fe_side->get_phi();
  const std::vector<std::vector<RealGradient>> & grad_phi = fe_side->get_dphi();

  points[0] = p_info._closest_point_ref;
  fe_side->reinit(side, &points);

  p_info._side_phi = phi;
  p_info._side_grad_phi = grad_phi;
  p_info._dxyzdxi = dxyz_dxi;
  p_info._dxyzdeta = dxyz_deta;
  p_info._d2xyzdxideta = d2xyz_dxieta;
}

void
restrictPointToFace(libMesh::Point & p,
                    const Elem * side,
                    std::vector<const Node *> & off_edge_nodes)
{
  const ElemType t(side->type());
  off_edge_nodes.clear();
  Real & xi = p(0);
  Real & eta = p(1);

  switch (t)
  {
    case EDGE2:
    case EDGE3:
    case EDGE4:
    {
      // The reference 1D element is [-1,1].
      if (xi < -1.0)
      {
        xi = -1.0;
        off_edge_nodes.push_back(side->node_ptr(0));
      }
      else if (xi > 1.0)
      {
        xi = 1.0;
        off_edge_nodes.push_back(side->node_ptr(1));
      }
      break;
    }

    case TRI3:
    case TRI6:
    case TRI7:
    {
      // The reference triangle is isosceles
      // and is bound by xi=0, eta=0, and xi+eta=1.

      if (xi <= 0.0 && eta <= 0.0)
      {
        xi = 0.0;
        eta = 0.0;
        off_edge_nodes.push_back(side->node_ptr(0));
      }
      else if (xi > 0.0 && xi < 1.0 && eta < 0.0)
      {
        eta = 0.0;
        off_edge_nodes.push_back(side->node_ptr(0));
        off_edge_nodes.push_back(side->node_ptr(1));
      }
      else if (eta > 0.0 && eta < 1.0 && xi < 0.0)
      {
        xi = 0.0;
        off_edge_nodes.push_back(side->node_ptr(2));
        off_edge_nodes.push_back(side->node_ptr(0));
      }
      else if (xi >= 1.0 && (eta - xi) <= -1.0)
      {
        xi = 1.0;
        eta = 0.0;
        off_edge_nodes.push_back(side->node_ptr(1));
      }
      else if (eta >= 1.0 && (eta - xi) >= 1.0)
      {
        xi = 0.0;
        eta = 1.0;
        off_edge_nodes.push_back(side->node_ptr(2));
      }
      else if ((xi + eta) > 1.0)
      {
        Real delta = (xi + eta - 1.0) / 2.0;
        xi -= delta;
        eta -= delta;
        off_edge_nodes.push_back(side->node_ptr(1));
        off_edge_nodes.push_back(side->node_ptr(2));
      }
      break;
    }

    case QUAD4:
    case QUAD8:
    case QUAD9:
    {
      // The reference quadrilateral element is [-1,1]^2.
      if (xi < -1.0)
      {
        xi = -1.0;
        if (eta < -1.0)
        {
          eta = -1.0;
          off_edge_nodes.push_back(side->node_ptr(0));
        }
        else if (eta > 1.0)
        {
          eta = 1.0;
          off_edge_nodes.push_back(side->node_ptr(3));
        }
        else
        {
          off_edge_nodes.push_back(side->node_ptr(3));
          off_edge_nodes.push_back(side->node_ptr(0));
        }
      }
      else if (xi > 1.0)
      {
        xi = 1.0;
        if (eta < -1.0)
        {
          eta = -1.0;
          off_edge_nodes.push_back(side->node_ptr(1));
        }
        else if (eta > 1.0)
        {
          eta = 1.0;
          off_edge_nodes.push_back(side->node_ptr(2));
        }
        else
        {
          off_edge_nodes.push_back(side->node_ptr(1));
          off_edge_nodes.push_back(side->node_ptr(2));
        }
      }
      else
      {
        if (eta < -1.0)
        {
          eta = -1.0;
          off_edge_nodes.push_back(side->node_ptr(0));
          off_edge_nodes.push_back(side->node_ptr(1));
        }
        else if (eta > 1.0)
        {
          eta = 1.0;
          off_edge_nodes.push_back(side->node_ptr(2));
          off_edge_nodes.push_back(side->node_ptr(3));
        }
      }
      break;
    }

    default:
    {
      mooseError("Unsupported face type: ", t);
      break;
    }
  }
}

} // namespace Moose
