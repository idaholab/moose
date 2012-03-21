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
#include "PenetrationThread.h"
#include "ParallelUniqueId.h"
#include "FindContactPoint.h"
#include "NearestNodeLocator.h"

// libmesh includes
#include "threads.h"

#include <algorithm>

//Temporary code until get_node_index gets added to libmesh Elem class:
unsigned int get_node_index (const Elem* el, const Node* nd)
{
  for (unsigned int n=0; n != el->n_nodes(); ++n)
    if (el->get_node(n) == nd)
      return n;

  return Node::invalid_id;
}


// Mutex to use when accessing _penetration_info;
Threads::spin_mutex pinfo_mutex;

PenetrationThread::PenetrationThread(const MeshBase & mesh,
                                     unsigned int master_boundary,
                                     unsigned int slave_boundary,
                                     std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & penetration_info,
                                     bool update_location,
                                     Real tangential_tolerance,
                                     std::vector<FEBase * > & fes,
                                     FEType & fe_type,
                                     NearestNodeLocator & nearest_node,
                                     std::vector<std::vector<unsigned int> > & node_to_elem_map,
                                     std::vector< unsigned int > & elem_list,
                                     std::vector< unsigned short int > & side_list,
                                     std::vector< short int > & id_list) :
  _mesh(mesh),
  _master_boundary(master_boundary),
  _slave_boundary(slave_boundary),
  _penetration_info(penetration_info),
  _update_location(update_location),
  _tangential_tolerance(tangential_tolerance),
  _fes(fes),
  _fe_type(fe_type),
  _nearest_node(nearest_node),
  _node_to_elem_map(node_to_elem_map),
  _elem_list(elem_list),
  _side_list(side_list),
  _id_list(id_list),
  _n_elems(elem_list.size())
{
}

// Splitting Constructor
PenetrationThread::PenetrationThread(PenetrationThread & x, Threads::split /*split*/) :
  _mesh(x._mesh),
  _master_boundary(x._master_boundary),
  _slave_boundary(x._slave_boundary),
  _penetration_info(x._penetration_info),
  _update_location(x._update_location),
  _tangential_tolerance(x._tangential_tolerance),
  _fes(x._fes),
  _fe_type(x._fe_type),
  _nearest_node(x._nearest_node),
  _node_to_elem_map(x._node_to_elem_map),
  _elem_list(x._elem_list),
  _side_list(x._side_list),
  _id_list(x._id_list),
  _n_elems(x._n_elems)
{
}

void
PenetrationThread::operator() (const NodeIdRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  FEBase * fe = _fes[_tid];

  for (NodeIdRange::const_iterator nd = range.begin() ; nd != range.end(); ++nd)
  {
    const Node & node = _mesh.node(*nd);

    // We're going to get a reference to the pointer for the pinfo for this node
    // This will alow us to manipulate this pointer without having to go through
    // the _penetration_info map... meaning this is the only mutex we'll have to do!
    pinfo_mutex.lock();
    PenetrationLocator::PenetrationInfo * & info = _penetration_info[node.id()];
    pinfo_mutex.unlock();

    // See if we already have info about this node
    if(info)
    {
      if (!_update_location && info->_distance >= 0)
      {
        const Point contact_ref = info->_closest_point_ref;
        const Real distance = info->_distance;
        bool contact_point_on_side(false);

        // Slave position must be the previous contact point
        // Use the previous reference coordinates
        std::vector<Point> points(1);
        points[0] = contact_ref;
        fe->reinit(info->_side, &points);
        const std::vector<Point> slave_pos = fe->get_xyz();
        Moose::findContactPoint(*info, fe, _fe_type, slave_pos[0],
                                false, _tangential_tolerance, contact_point_on_side);

        // Restore the original reference coordinates
        info->_closest_point_ref = contact_ref;
        info->_distance = distance;

        mooseAssert(info->_distance >= 0, "Error in PenetrationLocator: Slave node contained in element but contact distance was negative!");

        continue;
      }
      else
      {
        Real old_tangential_distance(info->_tangential_distance);
        bool contact_point_on_side(false);

        Moose::findContactPoint(*info, fe, _fe_type, node,
                                false, _tangential_tolerance, contact_point_on_side);

        if(contact_point_on_side)
        {
          if (info->_tangential_distance <= 0.0) //on the face
            continue;
          else if(info->_tangential_distance > 0.0 && old_tangential_distance > 0.0)
          { //off the face but within tolerance, was that way on the last step too
            Point dummy_closest_point(info->_closest_point_ref);
            std::vector<Node*>off_edge_nodes;
            Moose::restrictPointToFace(dummy_closest_point,
                                       info->_side,
                                       off_edge_nodes);
            if (off_edge_nodes.size()<2)
            { //Closest point is on a node rather than an edge.  It is possible that
              //another face is a better candidate.
              delete info;
              info = NULL;
            }
            else
              continue;
          }
          else
          {
            delete info;
            info = NULL;
          }
        }
        else
        {
          delete info;
          info = NULL;
        }
      }
    }

    const Node * closest_node = _nearest_node.nearestNode(node.id());
    std::vector<unsigned int> & closest_elems = _node_to_elem_map[closest_node->id()];
    std::vector<PenetrationLocator::PenetrationInfo*> p_info;

    for(unsigned int j=0; j<closest_elems.size(); j++)
    {
      unsigned int elem_id = closest_elems[j];
      Elem * elem = _mesh.elem(elem_id);

      // TODO: This is a horribly inefficient way to do this!  We need to cache information
      //       about which boundary ids elements are connected to and which sides are on those
      //       boundaries in MooseMesh!  That way we can look this information up directly!
      for(unsigned int m=0; m<_n_elems; ++m)
      {
        if(_elem_list[m] == elem_id && _id_list[m] == static_cast<short>(_master_boundary))
        {
          unsigned int side_num = _side_list[m];

          Elem *side = (elem->build_side(side_num,false)).release();

          Point contact_ref;
          Point contact_phys;
          Real distance = 0.;
          Real tangential_distance = 0.;
          RealGradient normal;
          bool contact_point_on_side;
          std::vector<Node*> off_edge_nodes;
          std::vector<std::vector<Real> > side_phi;
          std::vector<RealGradient> dxyzdxi;
          std::vector<RealGradient> dxyzdeta;
          std::vector<RealGradient> d2xyzdxideta;

          PenetrationLocator::PenetrationInfo * pen_info =
            new PenetrationLocator::PenetrationInfo(&node,
                                                    elem,
                                                    side,
                                                    side_num,
                                                    normal,
                                                    distance,
                                                    tangential_distance,
                                                    contact_phys,
                                                    contact_ref,
                                                    off_edge_nodes,
                                                    side_phi,
                                                    dxyzdxi,
                                                    dxyzdeta,
                                                    d2xyzdxideta);

          Moose::findContactPoint(*pen_info, fe, _fe_type, node,
                                  true, _tangential_tolerance, contact_point_on_side);

          if (contact_point_on_side)
          {
            if (!info)
            {
              info = pen_info;
            }
            else
            {
              CompeteInteractionResult CIResult=competeInteractions(pen_info,info);
              if (CIResult == FIRST_WINS){
                delete info;
                info = pen_info;
              }
              else if (CIResult == NEITHER_WINS){
                //Put both into p_info.  It is likely that the interaction will be on this edge.
                p_info.push_back(pen_info);
                p_info.push_back(info);
                info = NULL;
              }
            }
          }
          else
          {
            p_info.push_back( pen_info );
          }
        }
      }
    }

    // if we didn't find a match and there are two or more candidates...
    if (!info && p_info.size() > 1)
    {
      // The contact point is possibly on a ridge between two faces
      // or at a peak at the corner of multiple faces

      std::vector<Point>closest_points_ref(p_info.size());
      // Restrict the parametric coordinates to the domain of the face
      for ( unsigned int j(0); j < p_info.size(); ++j )
      {
        closest_points_ref[j] = p_info[j]->_closest_point_ref;
        Moose::restrictPointToFace(closest_points_ref[j],
                                   p_info[j]->_side,
                                   p_info[j]->_off_edge_nodes);
      }

      // Find the face for which the closest point to the projection
      // of the node on that face is closest to the node.
      unsigned int closest_index(0);
      Point closest_coor;
      Real dist(0);
      for ( unsigned int j(0); j < p_info.size(); ++j )
      {
        std::vector<Point> points(1);
        points[0] = closest_points_ref[j];
        fe->reinit(p_info[j]->_side, &points);
        const Point coor = fe->get_xyz()[0];
        Real dist2 = (coor - node).size();
        if (j==0 || dist2 < dist)
        {
          dist = dist2;
          closest_coor = coor;
          closest_index = j;
        }
      }

      // We now have the index for the closest face.
      for ( unsigned int j(0); j < p_info.size(); ++j )
      {

        if ( j != closest_index )
        {
          CommonEdgeResult offCommonEdge=interactionsOffCommonEdge(p_info[closest_index],p_info[j]);

          if (offCommonEdge != NO_COMMON)
          {
            bool found_contact_point(true);
            // The contact point is at the intersection between these two faces
            if (offCommonEdge == COMMON_EDGE)
            {
              //Nothing to do here.  closest_coor is the intersection point
              //TODO: thats's not completely true.  Run ridge routine to smooth between faces
            }
            else if (offCommonEdge == COMMON_NODE)
            {
              if (p_info[closest_index]->_elem->has_neighbor(p_info[j]->_elem)) //true only if they share a face
              {
                int interaction_to_keep(0);
                found_contact_point = findRidgeContactPoint(closest_coor,p_info[j],p_info[closest_index],interaction_to_keep);
              }
              // If false, they share only a node, so the contact point is on that node,
              // and closest_coor is the location of that point.
            }
            else if (offCommonEdge == EDGE_AND_COMMON_NODE)
            {
              int interaction_to_keep(0);
              found_contact_point = findRidgeContactPoint(closest_coor,p_info[j],p_info[closest_index],interaction_to_keep);
              if (interaction_to_keep==1)
              {
                info = p_info[j];
                p_info[j] = NULL; // Set this to NULL so that we don't delete it (now owned by _penetration_info).
                break;
              }
              else if (interaction_to_keep==2)
              {
                info = p_info[closest_index];
                p_info[closest_index] = NULL; // Set this to NULL so that we don't delete it (now owned by _penetration_info).
                break;
              }
            }
            if (found_contact_point)
            {
              // The contact point is at the intersection between these two faces
              p_info[closest_index]->_closest_point = closest_coor;
              p_info[closest_index]->_distance = (p_info[closest_index]->_distance >= 0 ? 1 : -1) * dist;
              Point normal(closest_coor - node);
              const Real len(normal.size());
              if (len > 0)
              {
                normal /= len;
              }
              const Real dot(normal * p_info[closest_index]->_normal);
              if (dot < 0)
              {
                normal *= -1;
              }
              p_info[closest_index]->_normal = normal;

              std::vector<Point> points(1);
              points[0] = p_info[closest_index]->_closest_point_ref;
              fe->reinit(p_info[closest_index]->_side, &points);
              p_info[closest_index]->_side_phi = fe->get_phi();
              p_info[closest_index]->_dxyzdxi = fe->get_dxyzdxi();
              p_info[closest_index]->_dxyzdeta = fe->get_dxyzdeta();
              p_info[closest_index]->_d2xyzdxideta = fe->get_d2xyzdxideta();

              info = p_info[closest_index];
              p_info[closest_index] = NULL; // Set this to NULL so that we don't delete it (now owned by _penetration_info).

              break;
            }
          }
        }
      }
    }
    for ( unsigned int j(0); j < p_info.size(); ++j )
    {
      delete p_info[j];
    }

  }
}

void
PenetrationThread::join(const PenetrationThread & /*other*/)
{}

//Is first interaction (pi1) stronger than second one (pi2)?
PenetrationThread::CompeteInteractionResult
PenetrationThread::competeInteractions(PenetrationLocator::PenetrationInfo * pi1,
                                       PenetrationLocator::PenetrationInfo * pi2)
{

  CompeteInteractionResult result(SECOND_WINS);

  //actually on face for i1, off face but within tangential tolerance for i2
  //always favor i1
  if (pi1->_tangential_distance == 0.0 && pi2->_tangential_distance > 0.0)
  {
    result=FIRST_WINS;
  }
  else if (pi2->_tangential_distance == 0.0 && pi1->_tangential_distance > 0.0)
  {
    result=SECOND_WINS;
  }
  else if (pi2->_tangential_distance > 0.0 &&
           pi1->_tangential_distance > 0.0 &&
           interactionsOffCommonEdge(pi1,pi2)!=NO_COMMON) //ridge case
  {
    result=NEITHER_WINS;
  }
  else
  {
    //i1 is closer, favor it no matter what
    if (std::abs(pi1->_distance) < std::abs(pi2->_distance))
    {
      result=FIRST_WINS;
    }

    //positive distance if penetrated, negative if off of face
    //i1 is penetrated, i2 isn't, so favor i1
    if (pi1->_distance >= 0.0 && pi2->_distance < 0.0)
    {
      result=FIRST_WINS;
    }
  }

  return result;
}

PenetrationThread::CommonEdgeResult
PenetrationThread::interactionsOffCommonEdge(PenetrationLocator::PenetrationInfo * pi1,
                                             PenetrationLocator::PenetrationInfo * pi2)
{
  CommonEdgeResult common_edge(NO_COMMON);
  const std::vector<Node*> &off_edge_nodes1(pi1->_off_edge_nodes);
  const std::vector<Node*> &off_edge_nodes2(pi2->_off_edge_nodes);
  const unsigned dim1(pi1->_side->dim());
  const unsigned dim2(pi2->_side->dim());

  if (dim1 == 1)
  {
    mooseAssert(dim2==1,"Incompatible dimensionalities");
    mooseAssert(off_edge_nodes1.size()<2 && off_edge_nodes2.size()<2,"off_edge_nodes size should be <2 for 2D contact");
    if (off_edge_nodes1.size()==1 && off_edge_nodes2.size()==1 && off_edge_nodes1[0]==off_edge_nodes2[0])
      common_edge = COMMON_EDGE;
  }
  else
  {
    mooseAssert(dim1==2 && dim2==2,"Incompatible dimensionalities");
    mooseAssert(off_edge_nodes1.size()<3 && off_edge_nodes2.size()<3,"off_edge_nodes size should be <3 for 3D contact");
    if (off_edge_nodes1.size()==1)
    {
      if (off_edge_nodes2.size()==1)
      {
        if (off_edge_nodes1[0] == off_edge_nodes2[0])
          common_edge = COMMON_NODE;
      }
      else if (off_edge_nodes2.size()==2)
      {
        if (off_edge_nodes1[0] == off_edge_nodes2[0] ||
            off_edge_nodes1[0] == off_edge_nodes2[1])
          common_edge = EDGE_AND_COMMON_NODE;
      }
    }
    else if (off_edge_nodes1.size()==2)
    {
      if (off_edge_nodes2.size()==1)
      {
        if (off_edge_nodes1[0] == off_edge_nodes2[0] ||
            off_edge_nodes1[1] == off_edge_nodes2[0])
          common_edge = EDGE_AND_COMMON_NODE;
      }
      else if (off_edge_nodes2.size()==2)
      {
        if ((off_edge_nodes1[0] == off_edge_nodes2[0] &&
             off_edge_nodes1[1] == off_edge_nodes2[1]) ||
            (off_edge_nodes1[1] == off_edge_nodes2[0] &&
             off_edge_nodes1[0] == off_edge_nodes2[1]))
          common_edge = COMMON_EDGE;
      }
    }
  }
  return common_edge;
}

bool
PenetrationThread::findRidgeContactPoint(Point &contact_point,
                                         PenetrationLocator::PenetrationInfo * pi1,
                                         PenetrationLocator::PenetrationInfo * pi2,
                                         int & interaction_to_keep)
{
  interaction_to_keep = 0;
  const unsigned sidedim(pi1->_side->dim());
  mooseAssert(sidedim == pi2->_side->dim(), "Incompatible dimensionalities");

  //Nodes on faces for the two interactions
  std::vector<Node*> side1_nodes;
  getSideCornerNodes(pi1->_side,side1_nodes);
  std::vector<Node*> side2_nodes;
  getSideCornerNodes(pi2->_side,side2_nodes);

  std::sort(side1_nodes.begin(),side1_nodes.end());
  std::sort(side2_nodes.begin(),side2_nodes.end());

  //Find nodes shared by the two faces
  std::vector<Node*> common_nodes;
  std::set_intersection(side1_nodes.begin(), side1_nodes.end(),
                       side2_nodes.begin(), side2_nodes.end(),
                       std::inserter(common_nodes, common_nodes.end()));

  if (common_nodes.size() != sidedim)
    return false;

  bool found_point1, found_point2;
  Point closest_coor_ref1(pi1->_closest_point_ref);
  Node* closest_node1;
  found_point1 = restrictPointToSpecifiedEdgeOfFace(closest_coor_ref1,closest_node1,pi1->_side,common_nodes);

  if (sidedim == 1)
  {
    if (!found_point1)
      return false;
  }
  else if (sidedim == 2)
  {
    Point closest_coor_ref2(pi2->_closest_point_ref);
    Node* closest_node2;
    found_point2 = restrictPointToSpecifiedEdgeOfFace(closest_coor_ref2,closest_node2,pi2->_side,common_nodes);
    if (!found_point1 && !found_point2)
      return false;
    else if (!found_point1) //interaction 1 isn't a ridge case.  Keep it if within tangential tolerance and don't move it to the ridge.
    {
      if (pi1->_tangential_distance <= _tangential_tolerance)
        interaction_to_keep = 1;
      return false;
    }
    else if (!found_point2) //interaction 2 isn't a ridge case.  Keep it if within tangential tolerance and don't move it to the ridge.
    {
      if (pi2->_tangential_distance <= _tangential_tolerance)
        interaction_to_keep = 2;
      return false;
    }

    //TODO:
    //We have the parametric coordinates of the closest intersection point for both faces.
    //We need to find a point somewhere in the middle of them so there's not an abrupt jump.
    //Find that point by taking dot products of vector from contact point to slave node point
    //with face normal vectors to see which face we're closer to.
  }

  FEBase * fe = _fes[_tid];
  std::vector<Point> points(1);
  points[0] = closest_coor_ref1;
  fe->reinit(pi1->_side, &points);
  contact_point = fe->get_xyz()[0];

  if (closest_node1)
  {
    RealGradient off_face = *closest_node1 - contact_point;
    Real tangential_distance = off_face.size();
    if (tangential_distance <= _tangential_tolerance)
      return true;
    else
      return false;
  }

  return true;
}

void
PenetrationThread::getSideCornerNodes(Elem* side,
                                      std::vector<Node*> &corner_nodes)
{
  const ElemType t(side->type());
  corner_nodes.clear();

  corner_nodes.push_back(side->get_node(0));
  corner_nodes.push_back(side->get_node(1));
  switch (t)
  {
    case EDGE2:
    case EDGE3:
    case EDGE4:
    {
      break;
    }

    case TRI3:
    case TRI6:
    {
      corner_nodes.push_back(side->get_node(2));
      break;
    }

    case QUAD4:
    case QUAD8:
    case QUAD9:
    {
      corner_nodes.push_back(side->get_node(2));
      corner_nodes.push_back(side->get_node(3));
      break;
    }

    default:
    {
      mooseError("Unsupported face type: "<<t);
      break;
    }
  }
}

bool
PenetrationThread::restrictPointToSpecifiedEdgeOfFace(Point& p,
                                                      Node* &closest_node,
                                                      const Elem* side,
                                                      const std::vector<Node*> &edge_nodes)
{
  const ElemType t(side->type());
  Real &xi   = p(0);
  Real &eta  = p(1);
  closest_node = NULL;

  std::vector<unsigned int> local_node_indices;
  for (unsigned int i(0); i<edge_nodes.size(); ++i)
  {
    //unsigned int local_index = side->get_node_index(edge_nodes[i]); //TODO: use this version once it makes it in libmesh
    unsigned int local_index = get_node_index(side,edge_nodes[i]);
    if (local_index == Node::invalid_id)
      mooseError("Side does not contain node");
    local_node_indices.push_back(local_index);
  }
  mooseAssert(local_node_indices.size() == side->dim(), "Number of edge nodes must match side dimensionality");
  std::sort(local_node_indices.begin(), local_node_indices.end());

  bool off_of_this_edge(false);

  switch (t)
  {
    case EDGE2:
    case EDGE3:
    case EDGE4:
    {
      if (local_node_indices[0] == 0)
      {
        if (xi <= -1.0)
        {
          xi = -1.0;
          off_of_this_edge=true;
          closest_node = side->get_node(0);
        }
      }
      else if (local_node_indices[0] == 1)
      {
        if (xi >= 1.0)
        {
          xi = 1.0;
          off_of_this_edge=true;
          closest_node = side->get_node(1);
        }
      }
      else
      {
        mooseError("Invalid local node indices");
      }
      break;
    }

    case TRI3:
    case TRI6:
    {
      if ((local_node_indices[0] == 0) &&
          (local_node_indices[1] == 1))
      {
        if (eta <= -1.0)
        {
          eta = -1.0;
          off_of_this_edge=true;
          if (xi<0.0)
            closest_node = side->get_node(0);
          else if (xi>1.0)
            closest_node = side->get_node(1);
        }
      }
      else if ((local_node_indices[0] == 1) &&
               (local_node_indices[1] == 2))
      {
        if ((xi + eta) > 1.0)
        {
          Real delta = (xi+eta-1.0)/2.0;
          xi -= delta;
          eta -= delta;
          off_of_this_edge=true;
          if (xi>1.0)
            closest_node = side->get_node(1);
          else if (xi<0.0)
            closest_node = side->get_node(2);
        }
      }
      else if ((local_node_indices[0] == 0) &&
               (local_node_indices[1] == 2))
      {
        if (xi <= -1.0)
        {
          xi = -1.0;
          off_of_this_edge=true;
          if (eta>1.0)
            closest_node = side->get_node(2);
          else if (eta<0.0)
            closest_node = side->get_node(0);
        }
      }
      else
      {
        mooseError("Invalid local node indices");
      }

      break;
    }

    case QUAD4:
    case QUAD8:
    case QUAD9:
    {
      if ((local_node_indices[0] == 0) &&
          (local_node_indices[1] == 1))
      {
        if (eta <= -1.0)
        {
          eta = -1.0;
          off_of_this_edge=true;
          if (xi<-1.0)
            closest_node = side->get_node(0);
          else if (xi>1.0)
            closest_node = side->get_node(1);
        }
      }
      else if ((local_node_indices[0] == 1) &&
               (local_node_indices[1] == 2))
      {
        if (xi >= 1.0)
        {
          xi = 1.0;
          off_of_this_edge=true;
          if (eta<-1.0)
            closest_node = side->get_node(1);
          else if (eta>1.0)
            closest_node = side->get_node(2);
        }
      }
      else if ((local_node_indices[0] == 2) &&
               (local_node_indices[1] == 3))
      {
        if (eta >= 1.0)
        {
          eta = 1.0;
          off_of_this_edge=true;
          if (xi<-1.0)
            closest_node = side->get_node(3);
          else if (xi>1.0)
            closest_node = side->get_node(2);
        }
      }
      else if ((local_node_indices[0] == 0) &&
               (local_node_indices[1] == 3))
      {
        if (xi <= -1.0)
        {
          xi = -1.0;
          off_of_this_edge=true;
          if (eta<-1.0)
            closest_node = side->get_node(0);
          else if (eta>1.0)
            closest_node = side->get_node(3);
        }
      }
      else
      {
        mooseError("Invalid local node indices");
      }
      break;
    }

    default:
    {
      mooseError("Unsupported face type: "<<t);
      break;
    }
  }
  return off_of_this_edge;
}
