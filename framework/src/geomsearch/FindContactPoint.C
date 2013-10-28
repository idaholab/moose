/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// Moose
#include "FindContactPoint.h"
#include "LineSegment.h"

// libMesh
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/plane.h"
#include "libmesh/fe_interface.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/fe_base.h"

namespace Moose
{

/**
 * Finds the closest point (called the contact point) on the master_elem on side "side" to the slave_point.
 *
 * @param master_elem The element on the master side of the interface
 * @param side The side number of the side of the master element
 * @param slave_point The physical space coordinates of the slave node
 * @param contact_ref The reference coordinate position of the contact point
 * @param contact_phys The physical space coordinates of the contact point
 * @param distance The distance between the slave_point and the contact point
 * @param normal The unit normal at the contact_point
 * @param contact_point_on_side whether or not the contact_point actually lies on _that_ side of the element.
 */
void
findContactPoint(PenetrationInfo & p_info,
                 FEBase * _fe, FEType & _fe_type, const Point & slave_point,
                 bool start_with_centroid, const Real tangential_tolerance,
                 bool & contact_point_on_side)
{
  const Elem * master_elem = p_info._elem;

  unsigned int dim = master_elem->dim();

  const Elem * side = p_info._side;

  const std::vector<Point> & phys_point = _fe->get_xyz();

  const std::vector<RealGradient> & dxyz_dxi = _fe->get_dxyzdxi();
  const std::vector<RealGradient> & d2xyz_dxi2 = _fe->get_d2xyzdxi2();
  const std::vector<RealGradient> & d2xyz_dxieta = _fe->get_d2xyzdxideta();

  const std::vector<RealGradient> & dxyz_deta = _fe->get_dxyzdeta();
  const std::vector<RealGradient> & d2xyz_deta2 = _fe->get_d2xyzdeta2();
  const std::vector<RealGradient> & d2xyz_detaxi = _fe->get_d2xyzdxideta();

  if (dim == 1)
  {
    Node * left(master_elem->get_node(0));
    Node * right(left);
    Real leftCoor((*left)(0));
    Real rightCoor(leftCoor);
    for (unsigned i(1); i < master_elem->n_nodes(); ++i)
    {
      Node * curr = master_elem->get_node(i);
      Real coor = (*curr)(0);
      if (coor < leftCoor)
      {
        left = curr;
        leftCoor = coor;
      }
      if (coor > rightCoor)
      {
        right = curr;
        rightCoor = coor;
      }
    }
    Node * nearestNode(left);
    Point nearestPoint(leftCoor, 0, 0);
    if (side->node(0) == right->id())
    {
      nearestNode = right;
      nearestPoint(0) = rightCoor;
    }
    else if (side->node(0) != left->id())
    {
      mooseError("Error findContactPoint.  Logic error in 1D");
    }
    p_info._closest_point_ref = FEInterface::inverse_map(dim, _fe_type, master_elem, nearestPoint, TOLERANCE, false);
    p_info._closest_point = nearestPoint;
    p_info._normal = Point(left == nearestNode ? -1 : 1, 0, 0);
    p_info._distance = (p_info._closest_point - slave_point) * p_info._normal;
    p_info._dxyzdxi = dxyz_dxi;
    p_info._dxyzdeta = dxyz_deta;
    p_info._d2xyzdxideta = d2xyz_dxieta;
    p_info._side_phi = _fe->get_phi();
    contact_point_on_side = true;
    return;
  }

  Point ref_point;

  if(start_with_centroid)
    ref_point = FEInterface::inverse_map(dim-1, _fe_type, side, side->centroid(), TOLERANCE, false);
  else
    ref_point = p_info._closest_point_ref;

  std::vector<Point> points(1);
  points[0] = ref_point;
  _fe->reinit(side, &points);
  RealGradient d = slave_point - phys_point[0];

  Real update_size = 9999999;

  //Least squares
  for(unsigned int it=0; it<3 && update_size > TOLERANCE*1e3; ++it)
  {

    DenseMatrix<Real> jac(dim-1, dim-1);

    jac(0,0) = -(dxyz_dxi[0] * dxyz_dxi[0]);

    if(dim-1 == 2)
    {
      jac(1,0) = -(dxyz_dxi[0] * dxyz_deta[0]);

      jac(0,1) = -(dxyz_deta[0] * dxyz_dxi[0]);
      jac(1,1) = -(dxyz_deta[0] * dxyz_deta[0]);
    }

    DenseVector<Real> rhs(dim-1);

    rhs(0) = dxyz_dxi[0]*d;

    if(dim-1 == 2)
      rhs(1) = dxyz_deta[0]*d;

    DenseVector<Real> update(dim-1);

    jac.lu_solve(rhs, update);

    ref_point(0) -= update(0);

    if(dim-1 == 2)
      ref_point(1) -= update(1);

    points[0] = ref_point;
    _fe->reinit(side, &points);
    d = slave_point - phys_point[0];

    update_size = update.l2_norm();
  }

  update_size = 9999999;

  unsigned nit=0;

  // Newton Loop
  for(; nit<12 && update_size > TOLERANCE*TOLERANCE; nit++)
  {
    d = slave_point - phys_point[0];

    DenseMatrix<Real> jac(dim-1, dim-1);

    jac(0,0) = (d2xyz_dxi2[0]*d)-(dxyz_dxi[0] * dxyz_dxi[0]);

    if(dim-1 == 2)
    {
      jac(1,0) = (d2xyz_dxieta[0]*d)-(dxyz_dxi[0] * dxyz_deta[0]);

      jac(0,1) = (d2xyz_detaxi[0]*d)-(dxyz_deta[0] * dxyz_dxi[0]);
      jac(1,1) = (d2xyz_deta2[0]*d)-(dxyz_deta[0] * dxyz_deta[0]);
    }

    DenseVector<Real> rhs(dim-1);

    rhs(0) = -dxyz_dxi[0]*d;

    if(dim-1 == 2)
      rhs(1) = -dxyz_deta[0]*d;

    DenseVector<Real> update(dim-1);

    jac.lu_solve(rhs, update);

    ref_point(0) += update(0);

    if(dim-1 == 2)
      ref_point(1) += update(1);

    points[0] = ref_point;
    _fe->reinit(side, &points);
    d = slave_point - phys_point[0];

    update_size = update.l2_norm();
  }

/*
  if(nit == 12 && update_size > TOLERANCE*TOLERANCE)
    Moose::err<<"Warning!  Newton solve for contact point failed to converge!"<<std::endl;
*/

  p_info._closest_point_ref = ref_point;
  p_info._closest_point = phys_point[0];
  p_info._distance = d.size();

  if(dim-1 == 2)
  {
    p_info._normal = dxyz_dxi[0].cross(dxyz_deta[0]);
    p_info._normal /= p_info._normal.size();
  }
  else
  {
    p_info._normal = RealGradient(dxyz_dxi[0](1),-dxyz_dxi[0](0));
    p_info._normal /= p_info._normal.size();
  }

  // If the point has not penetrated the face, make the distance negative
  const Real dot(d * p_info._normal);
  if (dot > 0.0)
    p_info._distance = -p_info._distance;

  contact_point_on_side = FEInterface::on_reference_element(ref_point, side->type());

  p_info._tangential_distance = 0.0;

  if (!contact_point_on_side)
  {
    p_info._closest_point_on_face_ref=ref_point;
    restrictPointToFace(p_info._closest_point_on_face_ref,side,p_info._off_edge_nodes);

    points[0] = p_info._closest_point_on_face_ref;
    _fe->reinit(side, &points);
    Point closest_point_on_face(phys_point[0]);

    RealGradient off_face = closest_point_on_face - p_info._closest_point;
    Real tangential_distance = off_face.size();
    p_info._tangential_distance = tangential_distance;
    if (tangential_distance <= tangential_tolerance)
    {
      contact_point_on_side = true;
    }
  }

  const std::vector<std::vector<Real> > & phi = _fe->get_phi();

  points[0] = p_info._closest_point_ref;
  _fe->reinit(side, &points);

  p_info._side_phi = phi;
  p_info._dxyzdxi = dxyz_dxi;
  p_info._dxyzdeta = dxyz_deta;
  p_info._d2xyzdxideta = d2xyz_dxieta;

}

void restrictPointToFace(Point& p,
                         const Elem* side,
                         std::vector<Node*> &off_edge_nodes)
{
  const ElemType t(side->type());
  off_edge_nodes.clear();
  Real &xi   = p(0);
  Real &eta  = p(1);

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
        off_edge_nodes.push_back(side->get_node(0));
      }
      else if (xi > 1.0)
      {
        xi = 1.0;
        off_edge_nodes.push_back(side->get_node(1));
      }
      break;
    }

    case TRI3:
    case TRI6:
    {
      // The reference triangle is isosceles
      // and is bound by xi=0, eta=0, and xi+eta=1.

      if (xi <= 0.0 && eta <= 0.0)
      {
        xi = 0.0;
        eta = 0.0;
        off_edge_nodes.push_back(side->get_node(0));
      }
      else if (xi > 0.0 && xi < 1.0
               && eta < 0.0)
      {
        eta = 0.0;
        off_edge_nodes.push_back(side->get_node(0));
        off_edge_nodes.push_back(side->get_node(1));
      }
      else if (eta > 0.0 && eta < 1.0
               && xi < 0.0)
      {
        xi = 0.0;
        off_edge_nodes.push_back(side->get_node(2));
        off_edge_nodes.push_back(side->get_node(0));
      }
      else if (xi >= 1.0
               && (eta - xi) <= -1.0)
      {
        xi = 1.0;
        eta = 0.0;
        off_edge_nodes.push_back(side->get_node(1));
      }
      else if (eta >= 1.0
               && (eta - xi) >= 1.0)
      {
        xi = 0.0;
        eta = 1.0;
        off_edge_nodes.push_back(side->get_node(2));
      }
      else if ((xi + eta) > 1.0)
      {
        Real delta = (xi+eta-1.0)/2.0;
        xi -= delta;
        eta -= delta;
        off_edge_nodes.push_back(side->get_node(1));
        off_edge_nodes.push_back(side->get_node(2));
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
          off_edge_nodes.push_back(side->get_node(0));
        }
        else if (eta > 1.0)
        {
          eta = 1.0;
          off_edge_nodes.push_back(side->get_node(3));
        }
        else
        {
          off_edge_nodes.push_back(side->get_node(3));
          off_edge_nodes.push_back(side->get_node(0));
        }
      }
      else if (xi > 1.0)
      {
        xi = 1.0;
        if (eta < -1.0)
        {
          eta = -1.0;
          off_edge_nodes.push_back(side->get_node(1));
        }
        else if (eta > 1.0)
        {
          eta = 1.0;
          off_edge_nodes.push_back(side->get_node(2));
        }
        else
        {
          off_edge_nodes.push_back(side->get_node(1));
          off_edge_nodes.push_back(side->get_node(2));
        }
      }
      else
      {
        if (eta < -1.0)
        {
          eta = -1.0;
          off_edge_nodes.push_back(side->get_node(0));
          off_edge_nodes.push_back(side->get_node(1));
        }
        else if (eta > 1.0)
        {
          eta = 1.0;
          off_edge_nodes.push_back(side->get_node(2));
          off_edge_nodes.push_back(side->get_node(3));
        }
      }
      break;
    }

    default:
    {
      mooseError("Unsupported face type: "<<t);
      break;
    }
  }
}


} //namespace Moose

