#include "PenetrationLocator.h"

#include "Moose.h"
#include "ArbitraryQuadrature.h"
#include "LineSegment.h"
#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"

#include "boundary_info.h"
#include "elem.h"
#include "plane.h"
#include "fe_interface.h"
#include "dense_matrix.h"
#include "dense_vector.h"

PenetrationLocator::PenetrationLocator(SubProblem & subproblem, GeometricSearchData & geom_search_data, MooseMesh & mesh, unsigned int master, unsigned int slave) :
    _subproblem(subproblem),
    _mesh(mesh),
    _master_boundary(master),
    _slave_boundary(slave),
    fe_type(),
    fe(FEBase::build(_mesh.dimension()-1, fe_type).release()),
    _nearest_node(geom_search_data.getNearestNodeLocator(master, slave))
{}

PenetrationLocator::~PenetrationLocator()
{
  delete fe;
}

void
PenetrationLocator::detectPenetration()
{
  Moose::perf_log.push("detectPenetration()","Solve");

  // Data structures to hold the element boundary information
  std::vector< unsigned int > elem_list;
  std::vector< unsigned short int > side_list;
  std::vector< short int > id_list;

  // Retrieve the Element Boundary data structures from the mesh
  _mesh.build_side_list(elem_list, side_list, id_list);

  // Data structures to hold the Nodal Boundary conditions
  std::vector< unsigned int > node_list;
  std::vector< short int > node_boundary_list;
//  _mesh.boundary_info->build_node_list_from_side_list();
  _mesh.build_node_list(node_list, node_boundary_list);

  const unsigned int n_nodes = node_list.size();
  const unsigned int n_elems = elem_list.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    unsigned int boundary_id = node_boundary_list[i];

    if(boundary_id == _slave_boundary)
    {
      Node & node = _mesh.node(node_list[i]);
/*      
      if(node.processor_id() == libMesh::processor_id())
      {
*/
        // See if we already have info about this node
        if(_penetration_info[node.id()])
        {
          PenetrationInfo * info = _penetration_info[node.id()];

          Elem * elem = info->_elem;
          Elem * side = info->_side;

          // See if the same element still contains this point
          if(elem->contains_point(node))
          {
            Point contact_ref = info->_closest_point_ref;
            Point contact_phys;
            Real distance;
            RealGradient normal;
            bool contact_point_on_side;
            std::vector<std::vector<Real> > side_phi;
  
            findContactPoint(elem, info->_side_num, node, false, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

//            info->_distance = normDistance(*elem, *side, node, closest_point, normal);

            info->_normal = normal;
            info->_closest_point_ref = contact_ref;
            info->_distance = distance;
            info->_side_phi = side_phi;
            
            mooseAssert(info->_distance >= 0, "Error in PenetrationLocator: Slave node contained in element but contact distance was negative!");
            
            info->_closest_point = contact_phys;

            // I hate continues but this is actually cleaner than anything I can think of
            continue;
          }
          else
          {
            // See if this element still has the same one across from it
//            Real distance = normDistance(*elem, *side, node, closest_point, normal);
            
            Point contact_ref = info->_closest_point_ref;
            Point contact_phys;
            Real distance;
            RealGradient normal;
            bool contact_point_on_side;
            std::vector<std::vector<Real> > side_phi;
  
            findContactPoint(elem, info->_side_num, node, false, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

            if(contact_point_on_side)
            {
              info->_normal = normal;
              info->_closest_point_ref = contact_ref;
              info->_distance = distance;
              mooseAssert(info->_distance <= 0, "Error in PenetrationLocator: Slave node not contained in element but distance was positive!");
              info->_side_phi = side_phi;

              info->_closest_point = contact_phys;

              continue;
            }
            else
            {
              delete _penetration_info[node.id()];
              _penetration_info[node.id()] = NULL;
            }
          }
        } 
          
        Node * closest_node = _nearest_node.nearestNode(node.id());
        
        Real closest_distance = _nearest_node.distance(node.id());
        /*
        for(unsigned int k=0; k<n_nodes; k++)  
        {
          unsigned int other_boundary_id = node_boundary_list[k];

          if(other_boundary_id == _master_boundary)
          {
            Node * cur_node = _mesh.node_ptr(node_list[k]);
                                             
            Real distance = ((*cur_node) - node).size();

            if(distance < closest_distance)
            {
              closest_distance = distance;
              closest_node = cur_node;
            }
          }
        }
        */
        std::vector<unsigned int> & slave_elems = _mesh.nodeToElemMap()[node.id()];

        bool a_slave_elem_on_this_processor = false;
        
        for(unsigned int j=0; j<slave_elems.size(); j++)
        {
          unsigned int elem_id = slave_elems[j];
          Elem * elem = _mesh.elem(elem_id);

          if(elem->processor_id() == libMesh::processor_id())
            a_slave_elem_on_this_processor = true;
        }

        std::vector<unsigned int> & closest_elems = _mesh.nodeToElemMap()[closest_node->id()];

        bool a_closest_elem_on_this_processor = false;
        
        for(unsigned int j=0; j<closest_elems.size(); j++)
        {
          unsigned int elem_id = closest_elems[j];
          Elem * elem = _mesh.elem(elem_id);

          if(elem->processor_id() == libMesh::processor_id())
            a_closest_elem_on_this_processor = true;
        }

        // If none of the potential interactions are on this processor... we don't need to worry
        // about this contact interaction
        if(!a_slave_elem_on_this_processor && !a_closest_elem_on_this_processor)
          continue;

        for(unsigned int j=0; j<closest_elems.size(); j++)
        {          
          unsigned int elem_id = closest_elems[j];
          Elem * elem = _mesh.elem(elem_id);
            
          for(unsigned int m=0; m<n_elems; m++)
          {
            if(elem_list[m] == elem_id && id_list[m] == _master_boundary)
            {
              unsigned int side_num = side_list[m];
              
              Elem *side = (elem->build_side(side_num)).release();
              
//              Real distance = normDistance(*elem, *side, node, closest_point, normal);
              Point contact_ref;
              Point contact_phys;
              Real distance;
              RealGradient normal;
              bool contact_point_on_side;
              std::vector<std::vector<Real> > side_phi;
  
              findContactPoint(elem, side_num, node, true, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

              if(contact_point_on_side && _penetration_info[node.id()] &&
                 (
                  (std::abs(_penetration_info[node.id()]->_distance) > std::abs(distance)) ||
                  (_penetration_info[node.id()]->_distance < 0 && distance > 0)
                 )
                )
              {
                delete _penetration_info[node.id()];
                _penetration_info[node.id()] = NULL;
              }

              if(contact_point_on_side && (!_penetration_info[node.id()] ||
                                           (
                                             (std::abs(_penetration_info[node.id()]->_distance) > std::abs(distance)) ||
                                             (_penetration_info[node.id()]->_distance < 0 && distance > 0)
                                           )
                                          )
                )
              {
                _penetration_info[node.id()] =  new PenetrationInfo(&node,
                                                                    elem,
                                                                    side,
                                                                    side_num,
                                                                    normal,
                                                                    distance,
                                                                    contact_phys,
                                                                    contact_ref,
                                                                    side_phi);
              }
              else
              {
                delete side;
              }
            }
          }
        }
//      }
    }
  }        

  Moose::perf_log.pop("detectPenetration()","Solve");
}

Real
PenetrationLocator::penetrationDistance(unsigned int node_id)
{
  PenetrationInfo * info = _penetration_info[node_id];
  
  if (info)
    return info->_distance;
  else
    return 0;
}


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
PenetrationLocator::findContactPoint(const Elem * master_elem, unsigned int side_num, const Point & slave_point,
                                     bool start_with_centroid, Point & contact_ref, Point & contact_phys, std::vector<std::vector<Real> > & side_phi,
                                     Real & distance, RealGradient & normal, bool & contact_point_on_side)
{
  unsigned int dim = master_elem->dim();
  
  Elem * side = master_elem->build_side(side_num, false).release();

//  FEType fe_type;
//  FEBase * fe = FEBase::build(dim-1, fe_type).release();

  const std::vector<Point> & phys_point = fe->get_xyz();

  const std::vector<RealGradient> & dxyz_dxi = fe->get_dxyzdxi();
  const std::vector<RealGradient> & d2xyz_dxi2 = fe->get_d2xyzdxi2();
  const std::vector<RealGradient> & d2xyz_dxieta = fe->get_d2xyzdxideta();
  
  const std::vector<RealGradient> & dxyz_deta = fe->get_dxyzdeta();
  const std::vector<RealGradient> & d2xyz_deta2 = fe->get_d2xyzdeta2();
  const std::vector<RealGradient> & d2xyz_detaxi = fe->get_d2xyzdxideta();
  
  Point ref_point;

  if(start_with_centroid)
    ref_point = FEInterface::inverse_map(dim-1, fe_type, side, side->centroid());
  else
    ref_point = contact_ref;

  std::vector<Point> points(1);
  points[0] = ref_point;
  fe->reinit(side, &points);
  RealGradient d = slave_point - phys_point[0];

  Real update_size = 9999999;

  //Least squares
  for(unsigned int it=0; it<3 && update_size > TOLERANCE*1e3; it++)
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
    fe->reinit(side, &points);
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
    fe->reinit(side, &points);
    d = slave_point - phys_point[0];

    update_size = update.l2_norm();
  }

  if(nit == 12 && update_size > TOLERANCE*TOLERANCE)
    std::cerr<<"Warning!  Newton solve for contact point failed to converge!"<<std::endl;

  bool contained_in_elem = master_elem->contains_point(slave_point);
  
  contact_ref = ref_point;
  contact_phys = phys_point[0];
  distance = d.size();

  // If the point is outside the element the distance is negative
  if(!contained_in_elem)
    distance = -distance;

  if(dim-1 == 2)
  {
    normal = dxyz_dxi[0].cross(dxyz_deta[0]);
    normal /= normal.size();
  }
  else
  {
    normal = RealGradient(dxyz_dxi[0](1),-dxyz_dxi[0](0));
    normal /= normal.size();
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
        AutoPtr<Elem> sideside = side->build_side(ss);

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

    contact_ref = FEInterface::inverse_map(dim-1, fe_type, side, closest_point);
    contact_phys = closest_point;
    distance = closest_distance;
    normal =  closest_point - slave_point;
    normal /= normal.size();
    contact_point_on_side = true;
  }

  // This happens if the point is behind the face, but through the element
  if(!contained_in_elem && ((contact_phys - slave_point) * normal) > 0)
    contact_point_on_side = false;

  
  const std::vector<std::vector<Real> > & phi = fe->get_phi();

  points[0] = contact_ref;
  fe->reinit(side, &points);

  side_phi = phi;

  delete side;
//  delete fe;
}

Real
PenetrationLocator::normDistance(const Elem & elem, const Elem & side, const Node & p0, Point & closest_point, RealVectorValue & normal)
{
  Real d;
  unsigned int dim = _mesh.dimension();

  Point p1 = side.point(0);    
  Point p2 = side.point(1);
  Point p3;
  
  if (dim == 2)
  {
    //Create a point that is just off in the z axis.
    //I do it this way so that it is about the same order of magnitude as the points themselves
    //TODO: Think about this some more!  Maybe need some absolute values.
    p3 = p1 + Point(0,0,p1(0)+p2(0)+p1(1)+p2(1));
  }
  else if (dim == 3)
    p3 = side.point(2);

  Plane p = Plane(p1, p2, p3);

  closest_point = p.closest_point(p0);

  // Make sure that the point is in the xy plane
  if(dim == 2)
    closest_point(2) = 0;
/*
  if(p0.id() == 7207)
  {
    std::cout<<"Node in elem:          "<<elem.contains_point(p0)<<std::endl;
    std::cout<<"Closest point in elem: "<<elem.contains_point(closest_point)<<std::endl;
    std::cout<<"Node:        "<<p0<<std::endl;
    std::cout<<"Closest Point"<<closest_point<<std::endl;
  }
*/

  if(elem.contains_point(closest_point))
  {
    d = (p0 - p.closest_point(p0)).size();
    if(p.above_surface(p0))
      d = -d;
    normal = p.unit_normal(closest_point);

//    RealVectorValue normal2 = -closest_point;

//    normal2(2) = 0.0;

//    normal2 /= normal2.size();

//    std::cout<<closest_point<<std::endl;
//    std::cout<<normal<<normal2<<std::endl;

//    normal = normal2;

    /********* Computes an Average Normal in 2D *************/

    if(dim == 2)
    {    
      Real dedge = 9999999999;
      unsigned int side_node_num = 0;
    
      for(unsigned int n=0; n<side.n_nodes(); n++)
      {
        Real cur_distance = (p0 - side.point(n)).size();
      
        if(cur_distance < dedge)
        {
          dedge = cur_distance;
          side_node_num = n;
        }
      }

      std::map<unsigned int, unsigned int> elems_to_sides;

      Real blend_length = side.hmax()*2e-1;
    
      if(dedge < blend_length)
      {
        Node * node = side.get_node(side_node_num);
        unsigned int node_id = node->id();

        std::vector<unsigned int> & elems_connected_to_node = _mesh.nodeToElemMap()[node_id];

        for(unsigned int i=0; i< elems_connected_to_node.size(); i++)
        {
          Elem * connected_elem = _mesh.elem(elems_connected_to_node[i]);

          elems_to_sides[connected_elem->id()] = _mesh.side_with_boundary_id(connected_elem, _master_boundary);
        }
      
        std::map<unsigned int, unsigned int>::iterator elems_it = elems_to_sides.begin();
        std::map<unsigned int, unsigned int>::iterator elems_end = elems_to_sides.end();

        Point neighbor_normal;
        Point my_normal;
      
        for(; elems_it != elems_end; ++elems_it)
        {
          Elem * connected_elem = _mesh.elem(elems_it->first);
        
          FEType fe_type;
          AutoPtr<FEBase> fe(FEBase::build(_mesh.dimension(), fe_type));
          ArbitraryQuadrature arbitrary_qrule(_mesh.dimension()-1, _subproblem.getQuadratureOrder());
          fe->attach_quadrature_rule(&arbitrary_qrule);
          const std::vector<Point>& normals = fe->get_normals();
      
          { 
            Point mapped = libMesh::FEInterface::inverse_map (2, fe_type, connected_elem, *node);
            std::vector<Point> mapped_points;
            mapped_points.push_back(mapped);
            arbitrary_qrule.setPoints(mapped_points);
            fe->reinit(connected_elem, elems_it->second);
          }      

          if(connected_elem->id() == elem.id())
            my_normal = normals[0];
          else
            neighbor_normal = normals[0];
        }
      
        Real theta = (-0.5*(1.0/blend_length)*dedge)+0.5;
        normal = (neighbor_normal*theta)+(my_normal*(1-theta));
      }
    }
  }
  /*
  else if(elem.contains_point(p0))  // If the point is in the element but the plane point wasn't...
  {
    std::cout<<"junk!"<<std::endl;
    
    d = 9999999999;
    unsigned int neighbor_num = 0;
    for(unsigned int n=0; n<side.n_nodes(); n++)
    {
      Real cur_distance = (p0 - side.point(n)).size();
      if(cur_distance < d)
      {
        d = cur_distance;
        closest_point = side.point(n);
        neighbor_num = n;
        normal = closest_point - p0;
        normal /= normal.size();
      }
    }
  */

/*
//      std::cout<<"--"<<std::endl<<dedge<<std::endl<<elem.neighbor(neighbor_num)->id()<<std::endl<<elem.id()<<std::endl;
      Elem * neighbor = elem.neighbor(neighbor_num);

      unsigned int node_id = side.node(neighbor_num);
      Node * node = side.get_node(neighbor_num);

//      std::cout<<node->id()<<std::endl;
      
      

      unsigned int nside = 0;

      for(unsigned int ns=0; ns<neighbor->n_sides(); ns++)
      {
        if(neighbor->neighbor(ns) == NULL)
          nside = ns;
      }


//      std::cout<<nside<<std::endl;
      nside = 3;
      
      AutoPtr<Elem> neighbor_side(neighbor->build_side(nside));
      
      FEType fe_type;
      AutoPtr<FEBase> fe(FEBase::build(_moose_system.getDim(), fe_type));
      ArbitraryQuadrature arbitrary_qrule(_moose_system.getDim()-1, _moose_system._max_quadrature_order);
      fe->attach_quadrature_rule(&arbitrary_qrule);
      const std::vector<Point>& normals = fe->get_normals();
      
      { 
        Point mapped = libMesh::FEInterface::inverse_map (2, fe_type, neighbor, *node);
        std::vector<Point> mapped_points;
        mapped_points.push_back(mapped);
        arbitrary_qrule.setPoints(mapped_points);
        fe->reinit(neighbor, nside);
      }      

      Point neighbor_normal = normals[0];

      { 
        Point mapped = libMesh::FEInterface::inverse_map (2, fe_type, &elem, *node);
        std::vector<Point> mapped_points;
        mapped_points.push_back(mapped);
        arbitrary_qrule.setPoints(mapped_points);
        fe->reinit(&elem, nside);
      }      

      Point my_normal = normals[0];

//      std::cout<<"Neighbor: "<<neighbor_normal;
//      std::cout<<"Mine: "<<my_normal;


//      Real theta = (-0.5e4*dedge)+0.5;

      Real theta = 0.5;
      
      
//      std::cout<<"new: "<<(neighbor_normal*theta)+(my_normal*(1-theta))<<std::endl;
      normal = (neighbor_normal*theta)+(my_normal*(1-theta));

//      std::cout<<normal<<std::endl;
      
//      std::cout<<normal<<std::endl;
  }
*/
  else
  {
    d = 9999999999;
  }
  
  return d;
}


RealVectorValue
PenetrationLocator::penetrationNormal(unsigned int node_id)
{
  std::map<unsigned int, PenetrationInfo *>::const_iterator found_it;

  if ((found_it = _penetration_info.find(node_id)) != _penetration_info.end())
    return found_it->second->_normal;
  else
    return RealVectorValue(0, 0, 0);
}


PenetrationLocator::PenetrationInfo::PenetrationInfo(Node * node, Elem * elem, Elem * side, unsigned int side_num, RealVectorValue norm, Real norm_distance, const Point & closest_point, const Point & closest_point_ref, const std::vector<std::vector<Real> > & side_phi)
  :_node(node),
   _elem(elem),
   _side(side),
   _side_num(side_num),
   _normal(norm),
   _distance(norm_distance),
   _closest_point(closest_point),
   _closest_point_ref(closest_point_ref),
   _side_phi(side_phi)
{}

  
PenetrationLocator::PenetrationInfo::PenetrationInfo(const PenetrationInfo & p) :
    _node(p._node),
    _elem(p._elem),
    _side(p._side),
    _side_num(p._side_num),
    _normal(p._normal),
    _distance(p._distance),
    _closest_point(p._closest_point),
    _closest_point_ref(p._closest_point_ref),
    _side_phi(p._side_phi)
{}

PenetrationLocator::PenetrationInfo::~PenetrationInfo()
{
  delete _side;
}   
