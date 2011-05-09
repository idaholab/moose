// Moose
#include "FindContactPoint.h"
#include "LineSegment.h"

// libMesh
#include "boundary_info.h"
#include "elem.h"
#include "plane.h"
#include "fe_interface.h"
#include "dense_matrix.h"
#include "dense_vector.h"
#include "fe_base.h"

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
findContactPoint(PenetrationLocator::PenetrationInfo & p_info,
                 FEBase * _fe, FEType & _fe_type, const Point & slave_point,
                 bool start_with_centroid, bool & contact_point_on_side)
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

  Point ref_point;

  if(start_with_centroid)
    ref_point = FEInterface::inverse_map(dim-1, _fe_type, side, side->centroid());
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
    RealGradient d = slave_point - phys_point[0];

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
    std::cerr<<"Warning!  Newton solve for contact point failed to converge!"<<std::endl;
*/
  bool contained_in_elem = master_elem->contains_point(slave_point);

  p_info._closest_point_ref = ref_point;
  p_info._closest_point = phys_point[0];
  p_info._distance = d.size();

  // If the point is outside the element the distance is negative
  if(!contained_in_elem)
    p_info._distance = -p_info._distance;

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

  contact_point_on_side = FEInterface::on_reference_element(ref_point, side->type());

  // This can happen if the element is distorted
  if(contained_in_elem && !contact_point_on_side)
  {
    Point closest_point;
    Real closest_distance = 999999;

    if(dim-1 == 2)
    {
      for(unsigned int ss=0; ss<side->n_sides(); ss++)
      {
        AutoPtr<Elem> sideside = side->build_side(ss,false);

        std::vector<Point> vertices;
        vertices.reserve(2);

        for(unsigned int ssn=0; ssn<sideside->n_nodes(); ssn++)
        {
          if(sideside->is_vertex(ssn))
            vertices.push_back(*(sideside->get_node(ssn)));
        }

        LineSegment ls(vertices[0], vertices[1]);

        Point cur_closest_point;

        bool on_segment = ls.closest_normal_point(slave_point, cur_closest_point);

        if(on_segment && (cur_closest_point - slave_point).size() < closest_distance)
        {
          closest_distance = (cur_closest_point - slave_point).size();
          closest_point = cur_closest_point;
        }
      }
    }
    else if(dim-1 == 1)
    {
      for(unsigned int sn=0; sn<side->n_nodes(); sn++)
      {
        Node * node = side->get_node(sn);

        if((slave_point-*node).size() < closest_distance)
        {
          closest_distance = (slave_point-*node).size();
          closest_point = *node;
        }
      }
    }

    p_info._closest_point_ref = FEInterface::inverse_map(dim-1, _fe_type, side, closest_point);
    p_info._closest_point = closest_point;
    p_info._distance = closest_distance;
    p_info._normal =  closest_point - slave_point;
    p_info._normal /= p_info._normal.size();
    contact_point_on_side = true;
  }

  // This happens if the point is behind the face, but through the element
  if(!contained_in_elem && ((p_info._closest_point - slave_point) * p_info._normal) > 0)
  {
    contact_point_on_side = false;
  }


  const std::vector<std::vector<Real> > & phi = _fe->get_phi();

  points[0] = p_info._closest_point_ref;
  _fe->reinit(side, &points);

  p_info._side_phi = phi;
  p_info._dxyzdxi = dxyz_dxi;
  p_info._dxyzdeta = dxyz_deta;
  p_info._d2xyzdxideta = d2xyz_dxieta;

}

} //namespace Moose

