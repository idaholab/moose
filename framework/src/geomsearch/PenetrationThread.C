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

// Mutex to use when accessing _penetration_info;
Threads::spin_mutex pinfo_mutex;

PenetrationThread::PenetrationThread(const MeshBase & mesh,
                                     BoundaryID master_boundary,
                                     BoundaryID slave_boundary,
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
          {
            continue;
          }
          else if(info->_tangential_distance > 0.0 && old_tangential_distance > 0.0)
          { //off the face but within tolerance, was that way on the last step too
            if (info->_side->dim()==2 && info->_off_edge_nodes.size()<2)
            { //Closest point on face is on a node rather than an edge.  Another
              //face might be a better candidate.
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

    mooseAssert(info==NULL,"Shouldn't have info at this point");

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

          Point contact_phys;
          Point contact_ref;
          Point contact_on_face_ref;
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
                                                    contact_on_face_ref,
                                                    off_edge_nodes,
                                                    side_phi,
                                                    dxyzdxi,
                                                    dxyzdeta,
                                                    d2xyzdxideta);

          Moose::findContactPoint(*pen_info, fe, _fe_type, node,
                                  true, _tangential_tolerance, contact_point_on_side);

          p_info.push_back( pen_info );

        }
      }
    }

    mooseAssert(info==NULL,"Shouldn't have info at this point");

    if (p_info.size() == 1)
    {
      if (p_info[0]->_tangential_distance <= _tangential_tolerance)
      {
        info = p_info[0];
        p_info[0] = NULL; // Set this to NULL so that we don't delete it (now owned by _penetration_info).
      }
    }
    else if (p_info.size() > 1)
    {

      //Loop through all pairs of faces, and check for contact on ridge betweeen each face pair
      std::vector<RidgeData> ridgeDataVec;
      for ( unsigned int i(0); i < p_info.size()-1; ++i )
      {
        for ( unsigned int j(i+1); j < p_info.size(); ++j )
        {
          Point closest_coor;
          Real tangential_distance(0.0);
          Node* closest_node(NULL);
          bool found_ridge_contact_point = findRidgeContactPoint(closest_coor,p_info[i],p_info[j],
                                                                 tangential_distance, closest_node);
          if (found_ridge_contact_point)
          {
            RidgeData rpd;
            rpd._index1=i;
            rpd._index2=j;
            rpd._closest_coor=closest_coor;
            rpd._tangential_distance=tangential_distance;
            rpd._closest_node=closest_node;
            ridgeDataVec.push_back(rpd);
          }
        }
      }
      if (ridgeDataVec.size() > 0) //find the ridge pair that is the best or find a peak
      {
        //Group together ridges for which we are off the edge of a common node.
        //Those are peaks.
        std::vector<RidgeSetData> ridgeSetDataVec;
        for (unsigned int i(0); i<ridgeDataVec.size(); ++i)
        {
          bool foundSetWithMatchingNode(false);
          for (unsigned int j(0); j<ridgeSetDataVec.size(); ++j)
          {
            if (ridgeDataVec[i]._closest_node != NULL &&
                ridgeDataVec[i]._closest_node == ridgeSetDataVec[j]._closest_node)
            {
              foundSetWithMatchingNode=true;
              ridgeSetDataVec[j]._ridge_data_vec.push_back(ridgeDataVec[i]);
              break;
            }
          }
          if (!foundSetWithMatchingNode)
          {
            RidgeSetData rsd;
            rsd._distance=std::numeric_limits<Real>::max();
            rsd._ridge_data_vec.push_back(ridgeDataVec[i]);
            rsd._closest_node=ridgeDataVec[i]._closest_node;
            ridgeSetDataVec.push_back(rsd);
          }
        }
        //Compute distance to each set of ridges
        for (unsigned int i(0); i<ridgeSetDataVec.size(); ++i)
        {
          if (ridgeSetDataVec[i]._closest_node != NULL) //peak or off edge of single ridge
          {
            if (ridgeSetDataVec[i]._ridge_data_vec.size() == 1) //off edge of single ridge
            {
              if (ridgeSetDataVec[i]._ridge_data_vec[0]._tangential_distance <= _tangential_tolerance) //off within tolerance
              {
                ridgeSetDataVec[i]._closest_coor = ridgeSetDataVec[i]._ridge_data_vec[0]._closest_coor;
                Point contact_point_vec = node - ridgeSetDataVec[i]._closest_coor;
                ridgeSetDataVec[i]._distance = contact_point_vec.size();
              }
            }
            else //several ridges join at common node to make peak.  The common node is the contact point
            {
              ridgeSetDataVec[i]._closest_coor = *ridgeSetDataVec[i]._closest_node;
              Point contact_point_vec = node - ridgeSetDataVec[i]._closest_coor;
              ridgeSetDataVec[i]._distance = contact_point_vec.size();
            }
          }
          else //on a single ridge
          {
            ridgeSetDataVec[i]._closest_coor = ridgeSetDataVec[i]._ridge_data_vec[0]._closest_coor;
            Point contact_point_vec = node - ridgeSetDataVec[i]._closest_coor;
            ridgeSetDataVec[i]._distance = contact_point_vec.size();
          }
        }
        //Find the set of ridges closest to us.
        unsigned int closest_ridge_set_index(0);
        Real closest_distance(ridgeSetDataVec[0]._distance);
        Point closest_point(ridgeSetDataVec[0]._closest_coor);
        for (unsigned int i(1); i<ridgeSetDataVec.size(); ++i)
        {
          if (ridgeSetDataVec[i]._distance < closest_distance)
          {
            closest_ridge_set_index = i;
            closest_distance = ridgeSetDataVec[i]._distance;
            closest_point = ridgeSetDataVec[i]._closest_coor;
          }
        }

        if (closest_distance < std::numeric_limits<Real>::max()) //contact point is on the closest ridge set
        {
          //find the face in the ridge set with the smallest index, assign that one to the interaction
          unsigned int face_index(std::numeric_limits<unsigned int>::max());
          unsigned int ridge_data_index(0);
          for (unsigned int i(0); i<ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec.size(); ++i)
          {
            if (ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[i]._index1 < face_index)
            {
              face_index = ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[i]._index1;
              ridge_data_index=i;
            }
            if (ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[i]._index2 < face_index)
            {
              face_index = ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[i]._index2;
              ridge_data_index=i;
            }
          }

          mooseAssert(face_index < std::numeric_limits<unsigned int>::max(),"face_index invalid");

          p_info[face_index]->_closest_point = closest_point;
          p_info[face_index]->_distance = (p_info[face_index]->_distance >= 0.0 ? 1.0 : -1.0) * closest_distance;
          Point normal(closest_point - node);
          const Real len(normal.size());
          if (len > 0)
          {
            normal /= len;
          }
          const Real dot(normal * p_info[face_index]->_normal);
          if (dot < 0)
          {
            normal *= -1;
          }
          p_info[face_index]->_normal = normal;
          p_info[face_index]->_tangential_distance = 0.0;

          if (ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec.size()==1) //contact with a single ridge rather than a peak
          {
            p_info[face_index]->_tangential_distance = ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[0]._tangential_distance;
          }

          std::vector<Point> points(1);
          points[0] = p_info[face_index]->_closest_point_ref;
          fe->reinit(p_info[face_index]->_side, &points);
          p_info[face_index]->_side_phi = fe->get_phi();
          p_info[face_index]->_dxyzdxi = fe->get_dxyzdxi();
          p_info[face_index]->_dxyzdeta = fe->get_dxyzdeta();
          p_info[face_index]->_d2xyzdxideta = fe->get_d2xyzdxideta();

          info = p_info[face_index];
          p_info[face_index] = NULL; // Set this to NULL so that we don't delete it (now owned by _penetration_info).
        }
        else
        {//todo:remove invalid ridge cases so they don't mess up individual face competition????
        }
      }

      if (!info) //contact wasn't on a ridge -- compete individual interactions
      {
        unsigned int best(0),i(1);
        do{
          CompeteInteractionResult CIResult=competeInteractions(p_info[best],p_info[i]);
          if (CIResult == FIRST_WINS){
            i++;
          } else if (CIResult == SECOND_WINS){
            best = i;
            i++;
          } else if (CIResult == NEITHER_WINS){
            best = i+1;
            i+=2;
          }
        }
        while(i<p_info.size() && best<p_info.size());
        if (best < p_info.size())
        {
          info = p_info[best];
          p_info[best] = NULL; // Set this to NULL so that we don't delete it (now owned by _penetration_info).
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

//Determine whether first (pi1) or second (pi2) interaction is stronger
PenetrationThread::CompeteInteractionResult
PenetrationThread::competeInteractions(PenetrationLocator::PenetrationInfo * pi1,
                                       PenetrationLocator::PenetrationInfo * pi2)
{

  CompeteInteractionResult result;

  if (pi1->_tangential_distance > _tangential_tolerance &&
      pi2->_tangential_distance > _tangential_tolerance) //out of tol on both faces
  {
    result=NEITHER_WINS;
  }
  else if (pi1->_tangential_distance == 0.0 &&
      pi2->_tangential_distance > 0.0)                   //on face 1, off face 2
  {
    result=FIRST_WINS;
  }
  else if (pi2->_tangential_distance == 0.0 &&
           pi1->_tangential_distance > 0.0)              //on face 2, off face 1
  {
    result=SECOND_WINS;
  }
  else if (pi1->_tangential_distance <= _tangential_tolerance &&
           pi2->_tangential_distance > _tangential_tolerance) //in face 1 tol, out of face 2 tol
  {
    result=FIRST_WINS;
  }
  else if (pi2->_tangential_distance <= _tangential_tolerance &&
           pi1->_tangential_distance > _tangential_tolerance) //in face 2 tol, out of face 1 tol
  {
    result=SECOND_WINS;
  }
  else if (pi1->_tangential_distance == 0.0 &&
           pi2->_tangential_distance == 0.0)                  //on both faces
  {
    if (pi1->_distance >= 0.0 && pi2->_distance < 0.0)  //favor face with positive distance (penetrated)
    {
      result=FIRST_WINS;
    }
    else if (pi2->_distance >= 0.0 && pi1->_distance < 0.0)
    {
      result=SECOND_WINS;
    }
    else if (std::abs(pi1->_distance) < std::abs(pi2->_distance)) //otherwise, favor the closer face
    {
      result=FIRST_WINS;
    }
    else if (std::abs(pi2->_distance) < std::abs(pi1->_distance)) //otherwise, favor the closer face
    {
      result=SECOND_WINS;
    }
    else //completely equal.  Favor the one with a smaller element id (for repeatibility)
    {
      if (pi1->_elem->id()<pi2->_elem->id())
      {
        result=FIRST_WINS;
      }
      else
      {
        result=SECOND_WINS;
      }
    }
  }
  else if (pi1->_tangential_distance <= _tangential_tolerance &&
           pi2->_tangential_distance <= _tangential_tolerance) //off but within tol of both faces
  {
    CommonEdgeResult cer=interactionsOffCommonEdge(pi1,pi2);
    if (cer == COMMON_EDGE || cer == COMMON_NODE) //ridge case.
    {
      //We already checked for ridges, and it got rejected, so neither face must be valid
      result=NEITHER_WINS;
//      mooseError("Erroneously encountered ridge case");
    }
    else if (cer == EDGE_AND_COMMON_NODE) //off side of face, off corner of another face.  Favor the off-side face
    {
      if (pi1->_off_edge_nodes.size() == pi2->_off_edge_nodes.size())
      {
        result=NEITHER_WINS;
        mooseError("Invalid off_edge_nodes counts");
      }
      else if (pi1->_off_edge_nodes.size()==2)
      {
        result=FIRST_WINS;
      }
      else if (pi2->_off_edge_nodes.size()==2)
      {
        result=SECOND_WINS;
      }
      else
      {
        result=NEITHER_WINS;
        mooseError("Invalid off_edge_nodes counts");
      }
    }
    else //Use the same logic as in the on-face condition (above).  A little copy-paste can't hurt...
    {
      if (pi1->_distance >= 0.0 && pi2->_distance < 0.0)  //favor face with positive distance (penetrated)
      {
        result=FIRST_WINS;
      }
      else if (pi2->_distance >= 0.0 && pi1->_distance < 0.0)
      {
        result=SECOND_WINS;
      }
      else if (std::abs(pi1->_distance) < std::abs(pi2->_distance)) //otherwise, favor the closer face
      {
        result=FIRST_WINS;
      }
      else if (std::abs(pi2->_distance) < std::abs(pi1->_distance)) //otherwise, favor the closer face
      {
        result=SECOND_WINS;
      }
      else //completely equal.  Favor the one with a smaller element id (for repeatibility)
      {
        if (pi1->_elem->id()<pi2->_elem->id())
        {
          result=FIRST_WINS;
        }
        else
        {
          result=SECOND_WINS;
        }
      }
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
                                         Real & tangential_distance,
                                         Node* &closest_node)
{
  tangential_distance = 0.0;
  closest_node = NULL;
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

  Point closest_coor_ref2(pi2->_closest_point_ref);
  Node* closest_node2;
  found_point2 = restrictPointToSpecifiedEdgeOfFace(closest_coor_ref2,closest_node2,pi2->_side,common_nodes);

  if (!found_point1 || !found_point2)
    return false;

//  if (sidedim == 2)
//  {
    //TODO:
    //We have the parametric coordinates of the closest intersection point for both faces.
    //We need to find a point somewhere in the middle of them so there's not an abrupt jump.
    //Find that point by taking dot products of vector from contact point to slave node point
    //with face normal vectors to see which face we're closer to.
//  }

  FEBase * fe = _fes[_tid];
  std::vector<Point> points(1);
  points[0] = closest_coor_ref1;
  fe->reinit(pi1->_side, &points);
  contact_point = fe->get_xyz()[0];

  if (sidedim == 2)
  {
    if (closest_node1) //point is off the ridge between the two elements
    {
      mooseAssert(closest_node1 == closest_node2,"If off edge of ridge, closest node must be the same on both elements");
      closest_node = closest_node1;

      RealGradient off_face = *closest_node1 - contact_point;
      tangential_distance = off_face.size();
    }
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
    unsigned int local_index = side->get_node_index(edge_nodes[i]);
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
