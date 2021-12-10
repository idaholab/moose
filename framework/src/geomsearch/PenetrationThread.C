//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose
#include "PenetrationThread.h"
#include "ParallelUniqueId.h"
#include "FindContactPoint.h"
#include "NearestNodeLocator.h"
#include "SubProblem.h"
#include "MooseVariableFE.h"
#include "MooseMesh.h"
#include "MooseUtils.h"

#include "libmesh/threads.h"

#include <algorithm>

// Mutex to use when accessing _penetration_info;
Threads::spin_mutex pinfo_mutex;

PenetrationThread::PenetrationThread(
    SubProblem & subproblem,
    const MooseMesh & mesh,
    BoundaryID primary_boundary,
    BoundaryID secondary_boundary,
    std::map<dof_id_type, PenetrationInfo *> & penetration_info,
    bool check_whether_reasonable,
    bool update_location,
    Real tangential_tolerance,
    bool do_normal_smoothing,
    Real normal_smoothing_distance,
    PenetrationLocator::NORMAL_SMOOTHING_METHOD normal_smoothing_method,
    std::vector<std::vector<FEBase *>> & fes,
    FEType & fe_type,
    NearestNodeLocator & nearest_node,
    const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map,
    const std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> & bc_tuples)
  : _subproblem(subproblem),
    _mesh(mesh),
    _primary_boundary(primary_boundary),
    _secondary_boundary(secondary_boundary),
    _penetration_info(penetration_info),
    _check_whether_reasonable(check_whether_reasonable),
    _update_location(update_location),
    _tangential_tolerance(tangential_tolerance),
    _do_normal_smoothing(do_normal_smoothing),
    _normal_smoothing_distance(normal_smoothing_distance),
    _normal_smoothing_method(normal_smoothing_method),
    _nodal_normal_x(NULL),
    _nodal_normal_y(NULL),
    _nodal_normal_z(NULL),
    _fes(fes),
    _fe_type(fe_type),
    _nearest_node(nearest_node),
    _node_to_elem_map(node_to_elem_map),
    _bc_tuples(bc_tuples)
{
}

// Splitting Constructor
PenetrationThread::PenetrationThread(PenetrationThread & x, Threads::split /*split*/)
  : _subproblem(x._subproblem),
    _mesh(x._mesh),
    _primary_boundary(x._primary_boundary),
    _secondary_boundary(x._secondary_boundary),
    _penetration_info(x._penetration_info),
    _check_whether_reasonable(x._check_whether_reasonable),
    _update_location(x._update_location),
    _tangential_tolerance(x._tangential_tolerance),
    _do_normal_smoothing(x._do_normal_smoothing),
    _normal_smoothing_distance(x._normal_smoothing_distance),
    _normal_smoothing_method(x._normal_smoothing_method),
    _fes(x._fes),
    _fe_type(x._fe_type),
    _nearest_node(x._nearest_node),
    _node_to_elem_map(x._node_to_elem_map),
    _bc_tuples(x._bc_tuples)
{
}

void
PenetrationThread::operator()(const NodeIdRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // Must get the variables every time this is run because _tid can change
  if (_do_normal_smoothing &&
      _normal_smoothing_method == PenetrationLocator::NSM_NODAL_NORMAL_BASED)
  {
    _nodal_normal_x = &_subproblem.getStandardVariable(_tid, "nodal_normal_x");
    _nodal_normal_y = &_subproblem.getStandardVariable(_tid, "nodal_normal_y");
    _nodal_normal_z = &_subproblem.getStandardVariable(_tid, "nodal_normal_z");
  }

  for (const auto & node_id : range)
  {
    const Node & node = _mesh.nodeRef(node_id);

    // We're going to get a reference to the pointer for the pinfo for this node
    // This will allow us to manipulate this pointer without having to go through
    // the _penetration_info map... meaning this is the only mutex we'll have to do!
    pinfo_mutex.lock();
    PenetrationInfo *& info = _penetration_info[node.id()];
    pinfo_mutex.unlock();

    std::vector<PenetrationInfo *> p_info;
    bool info_set(false);

    // See if we already have info about this node
    if (info)
    {
      FEBase * fe_elem = _fes[_tid][info->_elem->dim()];
      FEBase * fe_side = _fes[_tid][info->_side->dim()];

      if (!_update_location && (info->_distance >= 0 || info->isCaptured()))
      {
        const Point contact_ref = info->_closest_point_ref;
        bool contact_point_on_side(false);

        // Secondary position must be the previous contact point
        // Use the previous reference coordinates
        std::vector<Point> points(1);
        points[0] = contact_ref;
        const std::vector<Point> & secondary_pos = fe_side->get_xyz();

        // Prerequest other data we'll need in findContactPoint
        fe_side->get_phi();
        fe_side->get_dphi();
        fe_side->get_dxyzdxi();
        fe_side->get_d2xyzdxi2();
        fe_side->get_d2xyzdxideta();
        fe_side->get_dxyzdeta();
        fe_side->get_d2xyzdeta2();
        fe_side->get_d2xyzdxideta();

        fe_side->reinit(info->_side, &points);
        Moose::findContactPoint(*info,
                                fe_elem,
                                fe_side,
                                _fe_type,
                                secondary_pos[0],
                                false,
                                _tangential_tolerance,
                                contact_point_on_side);

        // Restore the original reference coordinates
        info->_closest_point_ref = contact_ref;
        // Just calculated as the distance of the contact point off the surface (0).  Set to 0 to
        // avoid round-off.
        info->_distance = 0.0;
        info_set = true;
      }
      else
      {
        Real old_tangential_distance(info->_tangential_distance);
        bool contact_point_on_side(false);

        Moose::findContactPoint(*info,
                                fe_elem,
                                fe_side,
                                _fe_type,
                                node,
                                false,
                                _tangential_tolerance,
                                contact_point_on_side);

        if (contact_point_on_side)
        {
          if (info->_tangential_distance <= 0.0) // on the face
          {
            info_set = true;
          }
          else if (info->_tangential_distance > 0.0 && old_tangential_distance > 0.0)
          { // off the face but within tolerance, was that way on the last step too
            if (info->_side->dim() == 2 && info->_off_edge_nodes.size() < 2)
            { // Closest point on face is on a node rather than an edge.  Another
              // face might be a better candidate.
            }
            else
            {
              info_set = true;
            }
          }
        }
      }
    }

    if (!info_set)
    {
      const Node * closest_node = _nearest_node.nearestNode(node.id());
      auto node_to_elem_pair = _node_to_elem_map.find(closest_node->id());
      mooseAssert(node_to_elem_pair != _node_to_elem_map.end(),
                  "Missing entry in node to elem map");
      const std::vector<dof_id_type> & closest_elems = node_to_elem_pair->second;

      for (const auto & elem_id : closest_elems)
      {
        const Elem * elem = _mesh.elemPtr(elem_id);

        std::vector<PenetrationInfo *> thisElemInfo;
        std::vector<const Node *> nodesThatMustBeOnSide;
        nodesThatMustBeOnSide.push_back(closest_node);
        createInfoForElem(
            thisElemInfo, p_info, &node, elem, nodesThatMustBeOnSide, _check_whether_reasonable);
      }

      if (p_info.size() == 1)
      {
        if (p_info[0]->_tangential_distance <= _tangential_tolerance)
        {
          switchInfo(info, p_info[0]);
          info_set = true;
        }
      }
      else if (p_info.size() > 1)
      {

        // Loop through all pairs of faces, and check for contact on ridge betweeen each face pair
        std::vector<RidgeData> ridgeDataVec;
        for (unsigned int i = 0; i + 1 < p_info.size(); ++i)
          for (unsigned int j = i + 1; j < p_info.size(); ++j)
          {
            Point closest_coor;
            Real tangential_distance(0.0);
            const Node * closest_node_on_ridge = NULL;
            unsigned int index = 0;
            Point closest_coor_ref;
            bool found_ridge_contact_point = findRidgeContactPoint(closest_coor,
                                                                   tangential_distance,
                                                                   closest_node_on_ridge,
                                                                   index,
                                                                   closest_coor_ref,
                                                                   p_info,
                                                                   i,
                                                                   j);
            if (found_ridge_contact_point)
            {
              RidgeData rpd;
              rpd._closest_coor = closest_coor;
              rpd._tangential_distance = tangential_distance;
              rpd._closest_node = closest_node_on_ridge;
              rpd._index = index;
              rpd._closest_coor_ref = closest_coor_ref;
              ridgeDataVec.push_back(rpd);
            }
          }

        if (ridgeDataVec.size() > 0) // Either find the ridge pair that is the best or find a peak
        {
          // Group together ridges for which we are off the edge of a common node.
          // Those are peaks.
          std::vector<RidgeSetData> ridgeSetDataVec;
          for (unsigned int i = 0; i < ridgeDataVec.size(); ++i)
          {
            bool foundSetWithMatchingNode = false;
            for (unsigned int j = 0; j < ridgeSetDataVec.size(); ++j)
            {
              if (ridgeDataVec[i]._closest_node != NULL &&
                  ridgeDataVec[i]._closest_node == ridgeSetDataVec[j]._closest_node)
              {
                foundSetWithMatchingNode = true;
                ridgeSetDataVec[j]._ridge_data_vec.push_back(ridgeDataVec[i]);
                break;
              }
            }
            if (!foundSetWithMatchingNode)
            {
              RidgeSetData rsd;
              rsd._distance = std::numeric_limits<Real>::max();
              rsd._ridge_data_vec.push_back(ridgeDataVec[i]);
              rsd._closest_node = ridgeDataVec[i]._closest_node;
              ridgeSetDataVec.push_back(rsd);
            }
          }
          // Compute distance to each set of ridges
          for (unsigned int i = 0; i < ridgeSetDataVec.size(); ++i)
          {
            if (ridgeSetDataVec[i]._closest_node !=
                NULL) // Either a peak or off the edge of single ridge
            {
              if (ridgeSetDataVec[i]._ridge_data_vec.size() == 1) // off edge of single ridge
              {
                if (ridgeSetDataVec[i]._ridge_data_vec[0]._tangential_distance <=
                    _tangential_tolerance) // off within tolerance
                {
                  ridgeSetDataVec[i]._closest_coor =
                      ridgeSetDataVec[i]._ridge_data_vec[0]._closest_coor;
                  Point contact_point_vec = node - ridgeSetDataVec[i]._closest_coor;
                  ridgeSetDataVec[i]._distance = contact_point_vec.norm();
                }
              }
              else // several ridges join at common node to make peak.  The common node is the
                   // contact point
              {
                ridgeSetDataVec[i]._closest_coor = *ridgeSetDataVec[i]._closest_node;
                Point contact_point_vec = node - ridgeSetDataVec[i]._closest_coor;
                ridgeSetDataVec[i]._distance = contact_point_vec.norm();
              }
            }
            else // on a single ridge
            {
              ridgeSetDataVec[i]._closest_coor =
                  ridgeSetDataVec[i]._ridge_data_vec[0]._closest_coor;
              Point contact_point_vec = node - ridgeSetDataVec[i]._closest_coor;
              ridgeSetDataVec[i]._distance = contact_point_vec.norm();
            }
          }
          // Find the set of ridges closest to us.
          unsigned int closest_ridge_set_index(0);
          Real closest_distance(ridgeSetDataVec[0]._distance);
          Point closest_point(ridgeSetDataVec[0]._closest_coor);
          for (unsigned int i = 1; i < ridgeSetDataVec.size(); ++i)
          {
            if (ridgeSetDataVec[i]._distance < closest_distance)
            {
              closest_ridge_set_index = i;
              closest_distance = ridgeSetDataVec[i]._distance;
              closest_point = ridgeSetDataVec[i]._closest_coor;
            }
          }

          if (closest_distance <
              std::numeric_limits<Real>::max()) // contact point is on the closest ridge set
          {
            // find the face in the ridge set with the smallest index, assign that one to the
            // interaction
            unsigned int face_index(std::numeric_limits<unsigned int>::max());
            for (unsigned int i = 0;
                 i < ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec.size();
                 ++i)
            {
              if (ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[i]._index < face_index)
                face_index = ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[i]._index;
            }

            mooseAssert(face_index < std::numeric_limits<unsigned int>::max(),
                        "face_index invalid");

            p_info[face_index]->_closest_point = closest_point;
            p_info[face_index]->_distance =
                (p_info[face_index]->_distance >= 0.0 ? 1.0 : -1.0) * closest_distance;
            // Calculate the normal as the vector from the ridge to the point only if we're not
            // doing normal
            // smoothing.  Normal smoothing will average out the normals on its own.
            if (!_do_normal_smoothing)
            {
              Point normal(closest_point - node);
              const Real len(normal.norm());
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
            }
            p_info[face_index]->_tangential_distance = 0.0;

            Point closest_point_ref;
            if (ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec.size() ==
                1) // contact with a single ridge rather than a peak
            {
              p_info[face_index]->_tangential_distance =
                  ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[0]._tangential_distance;
              p_info[face_index]->_closest_point_ref =
                  ridgeSetDataVec[closest_ridge_set_index]._ridge_data_vec[0]._closest_coor_ref;
            }
            else
            { // peak
              const Node * closest_node_on_face;
              bool restricted = restrictPointToFace(p_info[face_index]->_closest_point_ref,
                                                    closest_node_on_face,
                                                    p_info[face_index]->_side);
              if (restricted)
              {
                if (closest_node_on_face != ridgeSetDataVec[closest_ridge_set_index]._closest_node)
                {
                  mooseError("Closest node when restricting point to face != closest node from "
                             "RidgeSetData");
                }
              }
            }

            FEBase * fe = _fes[_tid][p_info[face_index]->_side->dim()];
            std::vector<Point> points(1);
            points[0] = p_info[face_index]->_closest_point_ref;
            fe->reinit(p_info[face_index]->_side, &points);
            p_info[face_index]->_side_phi = fe->get_phi();
            p_info[face_index]->_side_grad_phi = fe->get_dphi();
            p_info[face_index]->_dxyzdxi = fe->get_dxyzdxi();
            p_info[face_index]->_dxyzdeta = fe->get_dxyzdeta();
            p_info[face_index]->_d2xyzdxideta = fe->get_d2xyzdxideta();

            switchInfo(info, p_info[face_index]);
            info_set = true;
          }
          else
          { // todo:remove invalid ridge cases so they don't mess up individual face competition????
          }
        }

        if (!info_set) // contact wasn't on a ridge -- compete individual interactions
        {
          unsigned int best(0), i(1);
          do
          {
            CompeteInteractionResult CIResult = competeInteractions(p_info[best], p_info[i]);
            if (CIResult == FIRST_WINS)
            {
              i++;
            }
            else if (CIResult == SECOND_WINS)
            {
              best = i;
              i++;
            }
            else if (CIResult == NEITHER_WINS)
            {
              best = i + 1;
              i += 2;
            }
          } while (i < p_info.size() && best < p_info.size());
          if (best < p_info.size())
          {
            switchInfo(info, p_info[best]);
            info_set = true;
          }
        }
      }
    }

    if (!info_set)
    {
      // If penetration is not detected within the saved patch, it is possible
      // that the secondary node has moved outside the saved patch. So, the patch
      // for the secondary nodes saved in _recheck_secondary_nodes has to be updated
      // and the penetration detection has to be re-run on the updated patch.

      _recheck_secondary_nodes.push_back(node_id);

      delete info;
      info = NULL;
    }
    else
    {
      smoothNormal(info, p_info);
      FEBase * fe = _fes[_tid][info->_side->dim()];
      computeSlip(*fe, *info);
    }

    for (unsigned int j = 0; j < p_info.size(); ++j)
    {
      if (p_info[j])
      {
        delete p_info[j];
        p_info[j] = NULL;
      }
    }
  }
}

void
PenetrationThread::join(const PenetrationThread & other)
{
  _recheck_secondary_nodes.insert(_recheck_secondary_nodes.end(),
                                  other._recheck_secondary_nodes.begin(),
                                  other._recheck_secondary_nodes.end());
}

void
PenetrationThread::switchInfo(PenetrationInfo *& info, PenetrationInfo *& infoNew)
{
  mooseAssert(infoNew != NULL, "infoNew object is null");
  if (info)
  {
    infoNew->_starting_elem = info->_starting_elem;
    infoNew->_starting_side_num = info->_starting_side_num;
    infoNew->_starting_closest_point_ref = info->_starting_closest_point_ref;
    infoNew->_incremental_slip = info->_incremental_slip;
    infoNew->_accumulated_slip = info->_accumulated_slip;
    infoNew->_accumulated_slip_old = info->_accumulated_slip_old;
    infoNew->_frictional_energy = info->_frictional_energy;
    infoNew->_frictional_energy_old = info->_frictional_energy_old;
    infoNew->_contact_force = info->_contact_force;
    infoNew->_contact_force_old = info->_contact_force_old;
    infoNew->_lagrange_multiplier = info->_lagrange_multiplier;
    infoNew->_lagrange_multiplier_slip = info->_lagrange_multiplier_slip;
    infoNew->_locked_this_step = info->_locked_this_step;
    infoNew->_stick_locked_this_step = info->_stick_locked_this_step;
    infoNew->_mech_status = info->_mech_status;
    infoNew->_mech_status_old = info->_mech_status_old;
  }
  else
  {
    infoNew->_starting_elem = infoNew->_elem;
    infoNew->_starting_side_num = infoNew->_side_num;
    infoNew->_starting_closest_point_ref = infoNew->_closest_point_ref;
  }
  delete info;
  info = infoNew;
  infoNew = NULL; // Set this to NULL so that we don't delete it (now owned by _penetration_info).
}

PenetrationThread::CompeteInteractionResult
PenetrationThread::competeInteractions(PenetrationInfo * pi1, PenetrationInfo * pi2)
{

  CompeteInteractionResult result = NEITHER_WINS;

  if (pi1->_tangential_distance > _tangential_tolerance &&
      pi2->_tangential_distance > _tangential_tolerance) // out of tol on both faces
    result = NEITHER_WINS;

  else if (pi1->_tangential_distance == 0.0 &&
           pi2->_tangential_distance > 0.0) // on face 1, off face 2
    result = FIRST_WINS;

  else if (pi2->_tangential_distance == 0.0 &&
           pi1->_tangential_distance > 0.0) // on face 2, off face 1
    result = SECOND_WINS;

  else if (pi1->_tangential_distance <= _tangential_tolerance &&
           pi2->_tangential_distance > _tangential_tolerance) // in face 1 tol, out of face 2 tol
    result = FIRST_WINS;

  else if (pi2->_tangential_distance <= _tangential_tolerance &&
           pi1->_tangential_distance > _tangential_tolerance) // in face 2 tol, out of face 1 tol
    result = SECOND_WINS;

  else if (pi1->_tangential_distance == 0.0 && pi2->_tangential_distance == 0.0) // on both faces
    result = competeInteractionsBothOnFace(pi1, pi2);

  else if (pi1->_tangential_distance <= _tangential_tolerance &&
           pi2->_tangential_distance <= _tangential_tolerance) // off but within tol of both faces
  {
    CommonEdgeResult cer = interactionsOffCommonEdge(pi1, pi2);
    if (cer == COMMON_EDGE || cer == COMMON_NODE) // ridge case.
    {
      // We already checked for ridges, and it got rejected, so neither face must be valid
      result = NEITHER_WINS;
      //      mooseError("Erroneously encountered ridge case");
    }
    else if (cer == EDGE_AND_COMMON_NODE) // off side of face, off corner of another face.  Favor
                                          // the off-side face
    {
      if (pi1->_off_edge_nodes.size() == pi2->_off_edge_nodes.size())
        mooseError("Invalid off_edge_nodes counts");

      else if (pi1->_off_edge_nodes.size() == 2)
        result = FIRST_WINS;

      else if (pi2->_off_edge_nodes.size() == 2)
        result = SECOND_WINS;

      else
        mooseError("Invalid off_edge_nodes counts");
    }
    else // The node projects to both faces within tangential tolerance.
      result = competeInteractionsBothOnFace(pi1, pi2);
  }

  return result;
}

PenetrationThread::CompeteInteractionResult
PenetrationThread::competeInteractionsBothOnFace(PenetrationInfo * pi1, PenetrationInfo * pi2)
{
  CompeteInteractionResult result = NEITHER_WINS;

  if (pi1->_distance >= 0.0 && pi2->_distance < 0.0)
    result = FIRST_WINS; // favor face with positive distance (penetrated) -- first in this case

  else if (pi2->_distance >= 0.0 && pi1->_distance < 0.0)
    result = SECOND_WINS; // favor face with positive distance (penetrated) -- second in this case

  // TODO: This logic below could cause an abrupt jump from one face to the other with small mesh
  //       movement.  If there is some way to smooth the transition, we should do it.
  else if (MooseUtils::relativeFuzzyLessThan(std::abs(pi1->_distance), std::abs(pi2->_distance)))
    result = FIRST_WINS; // otherwise, favor the closer face -- first in this case

  else if (MooseUtils::relativeFuzzyLessThan(std::abs(pi2->_distance), std::abs(pi1->_distance)))
    result = SECOND_WINS; // otherwise, favor the closer face -- second in this case

  else // Equal within tolerance.  Favor the one with a smaller element id (for repeatibility)
  {
    if (pi1->_elem->id() < pi2->_elem->id())
      result = FIRST_WINS;

    else
      result = SECOND_WINS;
  }

  return result;
}

PenetrationThread::CommonEdgeResult
PenetrationThread::interactionsOffCommonEdge(PenetrationInfo * pi1, PenetrationInfo * pi2)
{
  CommonEdgeResult common_edge(NO_COMMON);
  const std::vector<const Node *> & off_edge_nodes1 = pi1->_off_edge_nodes;
  const std::vector<const Node *> & off_edge_nodes2 = pi2->_off_edge_nodes;
  const unsigned dim1 = pi1->_side->dim();

  if (dim1 == 1)
  {
    mooseAssert(pi2->_side->dim() == 1, "Incompatible dimensions.");
    mooseAssert(off_edge_nodes1.size() < 2 && off_edge_nodes2.size() < 2,
                "off_edge_nodes size should be <2 for 2D contact");
    if (off_edge_nodes1.size() == 1 && off_edge_nodes2.size() == 1 &&
        off_edge_nodes1[0] == off_edge_nodes2[0])
      common_edge = COMMON_EDGE;
  }
  else
  {
    mooseAssert(dim1 == 2 && pi2->_side->dim() == 2, "Incompatible dimensions.");
    mooseAssert(off_edge_nodes1.size() < 3 && off_edge_nodes2.size() < 3,
                "off_edge_nodes size should be <3 for 3D contact");
    if (off_edge_nodes1.size() == 1)
    {
      if (off_edge_nodes2.size() == 1)
      {
        if (off_edge_nodes1[0] == off_edge_nodes2[0])
          common_edge = COMMON_NODE;
      }
      else if (off_edge_nodes2.size() == 2)
      {
        if (off_edge_nodes1[0] == off_edge_nodes2[0] || off_edge_nodes1[0] == off_edge_nodes2[1])
          common_edge = EDGE_AND_COMMON_NODE;
      }
    }
    else if (off_edge_nodes1.size() == 2)
    {
      if (off_edge_nodes2.size() == 1)
      {
        if (off_edge_nodes1[0] == off_edge_nodes2[0] || off_edge_nodes1[1] == off_edge_nodes2[0])
          common_edge = EDGE_AND_COMMON_NODE;
      }
      else if (off_edge_nodes2.size() == 2)
      {
        if ((off_edge_nodes1[0] == off_edge_nodes2[0] &&
             off_edge_nodes1[1] == off_edge_nodes2[1]) ||
            (off_edge_nodes1[1] == off_edge_nodes2[0] && off_edge_nodes1[0] == off_edge_nodes2[1]))
          common_edge = COMMON_EDGE;
      }
    }
  }
  return common_edge;
}

bool
PenetrationThread::findRidgeContactPoint(Point & contact_point,
                                         Real & tangential_distance,
                                         const Node *& closest_node,
                                         unsigned int & index,
                                         Point & contact_point_ref,
                                         std::vector<PenetrationInfo *> & p_info,
                                         const unsigned int index1,
                                         const unsigned int index2)
{
  tangential_distance = 0.0;
  closest_node = NULL;
  PenetrationInfo * pi1 = p_info[index1];
  PenetrationInfo * pi2 = p_info[index2];
  const unsigned sidedim(pi1->_side->dim());
  mooseAssert(sidedim == pi2->_side->dim(), "Incompatible dimensionalities");

  // Nodes on faces for the two interactions
  std::vector<const Node *> side1_nodes;
  getSideCornerNodes(pi1->_side, side1_nodes);
  std::vector<const Node *> side2_nodes;
  getSideCornerNodes(pi2->_side, side2_nodes);

  std::sort(side1_nodes.begin(), side1_nodes.end());
  std::sort(side2_nodes.begin(), side2_nodes.end());

  // Find nodes shared by the two faces
  std::vector<const Node *> common_nodes;
  std::set_intersection(side1_nodes.begin(),
                        side1_nodes.end(),
                        side2_nodes.begin(),
                        side2_nodes.end(),
                        std::inserter(common_nodes, common_nodes.end()));

  if (common_nodes.size() != sidedim)
    return false;

  bool found_point1, found_point2;
  Point closest_coor_ref1(pi1->_closest_point_ref);
  const Node * closest_node1;
  found_point1 = restrictPointToSpecifiedEdgeOfFace(
      closest_coor_ref1, closest_node1, pi1->_side, common_nodes);

  Point closest_coor_ref2(pi2->_closest_point_ref);
  const Node * closest_node2;
  found_point2 = restrictPointToSpecifiedEdgeOfFace(
      closest_coor_ref2, closest_node2, pi2->_side, common_nodes);

  if (!found_point1 || !found_point2)
    return false;

  //  if (sidedim == 2)
  //  {
  // TODO:
  // We have the parametric coordinates of the closest intersection point for both faces.
  // We need to find a point somewhere in the middle of them so there's not an abrupt jump.
  // Find that point by taking dot products of vector from contact point to secondary node point
  // with face normal vectors to see which face we're closer to.
  //  }

  FEBase * fe = NULL;
  std::vector<Point> points(1);

  // We have to pick one of the two faces to own the contact point.  It doesn't really matter
  // which one we pick.  For repeatibility, pick the face with the lowest index.
  if (index1 < index2)
  {
    fe = _fes[_tid][pi1->_side->dim()];
    contact_point_ref = closest_coor_ref1;
    points[0] = closest_coor_ref1;
    fe->reinit(pi1->_side, &points);
    index = index1;
  }
  else
  {
    fe = _fes[_tid][pi2->_side->dim()];
    contact_point_ref = closest_coor_ref2;
    points[0] = closest_coor_ref2;
    fe->reinit(pi2->_side, &points);
    index = index2;
  }

  contact_point = fe->get_xyz()[0];

  if (sidedim == 2)
  {
    if (closest_node1) // point is off the ridge between the two elements
    {
      mooseAssert((closest_node1 == closest_node2 || closest_node2 == NULL),
                  "If off edge of ridge, closest node must be the same on both elements");
      closest_node = closest_node1;

      RealGradient off_face = *closest_node1 - contact_point;
      tangential_distance = off_face.norm();
    }
  }

  return true;
}

void
PenetrationThread::getSideCornerNodes(const Elem * side, std::vector<const Node *> & corner_nodes)
{
  const ElemType t(side->type());
  corner_nodes.clear();

  corner_nodes.push_back(side->node_ptr(0));
  corner_nodes.push_back(side->node_ptr(1));
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
      corner_nodes.push_back(side->node_ptr(2));
      break;
    }

    case QUAD4:
    case QUAD8:
    case QUAD9:
    {
      corner_nodes.push_back(side->node_ptr(2));
      corner_nodes.push_back(side->node_ptr(3));
      break;
    }

    default:
    {
      mooseError("Unsupported face type: ", t);
      break;
    }
  }
}

bool
PenetrationThread::restrictPointToSpecifiedEdgeOfFace(Point & p,
                                                      const Node *& closest_node,
                                                      const Elem * side,
                                                      const std::vector<const Node *> & edge_nodes)
{
  const ElemType t = side->type();
  Real & xi = p(0);
  Real & eta = p(1);
  closest_node = NULL;

  std::vector<unsigned int> local_node_indices;
  for (const auto & edge_node : edge_nodes)
  {
    unsigned int local_index = side->get_node_index(edge_node);
    if (local_index == libMesh::invalid_uint)
      mooseError("Side does not contain node");
    local_node_indices.push_back(local_index);
  }
  mooseAssert(local_node_indices.size() == side->dim(),
              "Number of edge nodes must match side dimensionality");
  std::sort(local_node_indices.begin(), local_node_indices.end());

  bool off_of_this_edge = false;

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
          off_of_this_edge = true;
          closest_node = side->node_ptr(0);
        }
      }
      else if (local_node_indices[0] == 1)
      {
        if (xi >= 1.0)
        {
          xi = 1.0;
          off_of_this_edge = true;
          closest_node = side->node_ptr(1);
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
      if ((local_node_indices[0] == 0) && (local_node_indices[1] == 1))
      {
        if (eta <= 0.0)
        {
          eta = 0.0;
          off_of_this_edge = true;
          if (xi < 0.0)
            closest_node = side->node_ptr(0);
          else if (xi > 1.0)
            closest_node = side->node_ptr(1);
        }
      }
      else if ((local_node_indices[0] == 1) && (local_node_indices[1] == 2))
      {
        if ((xi + eta) > 1.0)
        {
          Real delta = (xi + eta - 1.0) / 2.0;
          xi -= delta;
          eta -= delta;
          off_of_this_edge = true;
          if (xi > 1.0)
            closest_node = side->node_ptr(1);
          else if (xi < 0.0)
            closest_node = side->node_ptr(2);
        }
      }
      else if ((local_node_indices[0] == 0) && (local_node_indices[1] == 2))
      {
        if (xi <= 0.0)
        {
          xi = 0.0;
          off_of_this_edge = true;
          if (eta > 1.0)
            closest_node = side->node_ptr(2);
          else if (eta < 0.0)
            closest_node = side->node_ptr(0);
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
      if ((local_node_indices[0] == 0) && (local_node_indices[1] == 1))
      {
        if (eta <= -1.0)
        {
          eta = -1.0;
          off_of_this_edge = true;
          if (xi < -1.0)
            closest_node = side->node_ptr(0);
          else if (xi > 1.0)
            closest_node = side->node_ptr(1);
        }
      }
      else if ((local_node_indices[0] == 1) && (local_node_indices[1] == 2))
      {
        if (xi >= 1.0)
        {
          xi = 1.0;
          off_of_this_edge = true;
          if (eta < -1.0)
            closest_node = side->node_ptr(1);
          else if (eta > 1.0)
            closest_node = side->node_ptr(2);
        }
      }
      else if ((local_node_indices[0] == 2) && (local_node_indices[1] == 3))
      {
        if (eta >= 1.0)
        {
          eta = 1.0;
          off_of_this_edge = true;
          if (xi < -1.0)
            closest_node = side->node_ptr(3);
          else if (xi > 1.0)
            closest_node = side->node_ptr(2);
        }
      }
      else if ((local_node_indices[0] == 0) && (local_node_indices[1] == 3))
      {
        if (xi <= -1.0)
        {
          xi = -1.0;
          off_of_this_edge = true;
          if (eta < -1.0)
            closest_node = side->node_ptr(0);
          else if (eta > 1.0)
            closest_node = side->node_ptr(3);
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
      mooseError("Unsupported face type: ", t);
      break;
    }
  }
  return off_of_this_edge;
}

bool
PenetrationThread::restrictPointToFace(Point & p, const Node *& closest_node, const Elem * side)
{
  const ElemType t(side->type());
  Real & xi = p(0);
  Real & eta = p(1);
  closest_node = NULL;

  bool off_of_this_face(false);

  switch (t)
  {
    case EDGE2:
    case EDGE3:
    case EDGE4:
    {
      if (xi < -1.0)
      {
        xi = -1.0;
        off_of_this_face = true;
        closest_node = side->node_ptr(0);
      }
      else if (xi > 1.0)
      {
        xi = 1.0;
        off_of_this_face = true;
        closest_node = side->node_ptr(1);
      }
      break;
    }

    case TRI3:
    case TRI6:
    {
      if (eta < 0.0)
      {
        eta = 0.0;
        off_of_this_face = true;
        if (xi < 0.5)
        {
          closest_node = side->node_ptr(0);
          if (xi < 0.0)
            xi = 0.0;
        }
        else
        {
          closest_node = side->node_ptr(1);
          if (xi > 1.0)
            xi = 1.0;
        }
      }
      else if ((xi + eta) > 1.0)
      {
        Real delta = (xi + eta - 1.0) / 2.0;
        xi -= delta;
        eta -= delta;
        off_of_this_face = true;
        if (xi > 0.5)
        {
          closest_node = side->node_ptr(1);
          if (xi > 1.0)
          {
            xi = 1.0;
            eta = 0.0;
          }
        }
        else
        {
          closest_node = side->node_ptr(2);
          if (xi < 0.0)
          {
            xi = 0.0;
            eta = 1.0;
          }
        }
      }
      else if (xi < 0.0)
      {
        xi = 0.0;
        off_of_this_face = true;
        if (eta > 0.5)
        {
          closest_node = side->node_ptr(2);
          if (eta > 1.0)
            eta = 1.0;
        }
        else
        {
          closest_node = side->node_ptr(0);
          if (eta < 0.0)
            eta = 0.0;
        }
      }
      break;
    }

    case QUAD4:
    case QUAD8:
    case QUAD9:
    {
      if (eta < -1.0)
      {
        eta = -1.0;
        off_of_this_face = true;
        if (xi < 0.0)
        {
          closest_node = side->node_ptr(0);
          if (xi < -1.0)
            xi = -1.0;
        }
        else
        {
          closest_node = side->node_ptr(1);
          if (xi > 1.0)
            xi = 1.0;
        }
      }
      else if (xi > 1.0)
      {
        xi = 1.0;
        off_of_this_face = true;
        if (eta < 0.0)
        {
          closest_node = side->node_ptr(1);
          if (eta < -1.0)
            eta = -1.0;
        }
        else
        {
          closest_node = side->node_ptr(2);
          if (eta > 1.0)
            eta = 1.0;
        }
      }
      else if (eta > 1.0)
      {
        eta = 1.0;
        off_of_this_face = true;
        if (xi < 0.0)
        {
          closest_node = side->node_ptr(3);
          if (xi < -1.0)
            xi = -1.0;
        }
        else
        {
          closest_node = side->node_ptr(2);
          if (xi > 1.0)
            xi = 1.0;
        }
      }
      else if (xi < -1.0)
      {
        xi = -1.0;
        off_of_this_face = true;
        if (eta < 0.0)
        {
          closest_node = side->node_ptr(0);
          if (eta < -1.0)
            eta = -1.0;
        }
        else
        {
          closest_node = side->node_ptr(3);
          if (eta > 1.0)
            eta = 1.0;
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
  return off_of_this_face;
}

bool
PenetrationThread::isFaceReasonableCandidate(const Elem * primary_elem,
                                             const Elem * side,
                                             FEBase * fe,
                                             const Point * secondary_point,
                                             const Real tangential_tolerance)
{
  unsigned int dim = primary_elem->dim();

  const std::vector<Point> & phys_point = fe->get_xyz();

  const std::vector<RealGradient> & dxyz_dxi = fe->get_dxyzdxi();
  const std::vector<RealGradient> & dxyz_deta = fe->get_dxyzdeta();

  Point ref_point;

  std::vector<Point> points(1); // Default constructor gives us a point at 0,0,0

  fe->reinit(side, &points);

  RealGradient d = *secondary_point - phys_point[0];

  const Real twosqrt2 = 2.8284; // way more precision than we actually need here
  Real max_face_length = side->hmax() + twosqrt2 * tangential_tolerance;

  RealVectorValue normal;
  if (dim - 1 == 2)
  {
    normal = dxyz_dxi[0].cross(dxyz_deta[0]);
  }
  else if (dim - 1 == 1)
  {
    const Node * const * elem_nodes = primary_elem->get_nodes();
    const Point in_plane_vector1 = *elem_nodes[1] - *elem_nodes[0];
    const Point in_plane_vector2 = *elem_nodes[2] - *elem_nodes[0];

    Point out_of_plane_normal = in_plane_vector1.cross(in_plane_vector2);
    out_of_plane_normal /= out_of_plane_normal.norm();

    normal = dxyz_dxi[0].cross(out_of_plane_normal);
  }
  else
  {
    return true;
  }
  normal /= normal.norm();

  const Real dot(d * normal);

  const RealGradient normcomp = dot * normal;
  const RealGradient tangcomp = d - normcomp;

  const Real tangdist = tangcomp.norm();

  // Increase the size of the zone that we consider if the vector from the face
  // to the node has a larger normal component
  const Real faceExpansionFactor = 2.0 * (1.0 + normcomp.norm() / d.norm());

  bool isReasonableCandidate = true;
  if (tangdist > faceExpansionFactor * max_face_length)
  {
    isReasonableCandidate = false;
  }
  return isReasonableCandidate;
}

void
PenetrationThread::computeSlip(FEBase & fe, PenetrationInfo & info)
{
  // Slip is current projected position of secondary node minus
  //   original projected position of secondary node
  std::vector<Point> points(1);
  points[0] = info._starting_closest_point_ref;
  const auto & side = _elem_side_builder(*info._starting_elem, info._starting_side_num);
  fe.reinit(&side, &points);
  const std::vector<Point> & starting_point = fe.get_xyz();
  info._incremental_slip = info._closest_point - starting_point[0];
  if (info.isCaptured())
  {
    info._frictional_energy =
        info._frictional_energy_old + info._contact_force * info._incremental_slip;
    info._accumulated_slip = info._accumulated_slip_old + info._incremental_slip.norm();
  }
}

void
PenetrationThread::smoothNormal(PenetrationInfo * info, std::vector<PenetrationInfo *> & p_info)
{
  if (_do_normal_smoothing)
  {
    if (_normal_smoothing_method == PenetrationLocator::NSM_EDGE_BASED)
    {
      // If we are within the smoothing distance of any edges or corners, find the
      // corner nodes for those edges/corners, and weights from distance to edge/corner
      std::vector<Real> edge_face_weights;
      std::vector<PenetrationInfo *> edge_face_info;

      getSmoothingFacesAndWeights(info, edge_face_info, edge_face_weights, p_info);

      mooseAssert(edge_face_info.size() == edge_face_weights.size(),
                  "edge_face_info.size() != edge_face_weights.size()");

      if (edge_face_info.size() > 0)
      {
        // Smooth the normal using the weighting functions for all participating faces.
        RealVectorValue new_normal;
        Real this_face_weight = 1.0;

        for (unsigned int efwi = 0; efwi < edge_face_weights.size(); ++efwi)
        {
          PenetrationInfo * npi = edge_face_info[efwi];
          if (npi)
            new_normal += npi->_normal * edge_face_weights[efwi];

          this_face_weight -= edge_face_weights[efwi];
        }
        mooseAssert(this_face_weight >= (0.25 - 1e-8),
                    "Sum of weights of other faces shouldn't exceed 0.75");
        new_normal += info->_normal * this_face_weight;

        const Real len = new_normal.norm();
        if (len > 0)
          new_normal /= len;

        info->_normal = new_normal;
      }
    }
    else if (_normal_smoothing_method == PenetrationLocator::NSM_NODAL_NORMAL_BASED)
    {
      // params.addParam<VariableName>("var_name","description");
      // getParam<VariableName>("var_name")
      info->_normal(0) = _nodal_normal_x->getValue(info->_side, info->_side_phi);
      info->_normal(1) = _nodal_normal_y->getValue(info->_side, info->_side_phi);
      info->_normal(2) = _nodal_normal_z->getValue(info->_side, info->_side_phi);
      const Real len(info->_normal.norm());
      if (len > 0)
        info->_normal /= len;
    }
  }
}

void
PenetrationThread::getSmoothingFacesAndWeights(PenetrationInfo * info,
                                               std::vector<PenetrationInfo *> & edge_face_info,
                                               std::vector<Real> & edge_face_weights,
                                               std::vector<PenetrationInfo *> & p_info)
{
  const Elem * side = info->_side;
  const Point & p = info->_closest_point_ref;
  std::set<dof_id_type> elems_to_exclude;
  elems_to_exclude.insert(info->_elem->id());
  const Node * secondary_node = info->_node;

  std::vector<std::vector<const Node *>> edge_nodes;

  // Get the pairs of nodes along every edge that we are close enough to smooth with
  getSmoothingEdgeNodesAndWeights(p, side, edge_nodes, edge_face_weights);
  std::vector<Elem *> edge_neighbor_elems;
  edge_face_info.resize(edge_nodes.size(), NULL);

  std::vector<unsigned int> edges_without_neighbors;

  for (unsigned int i = 0; i < edge_nodes.size(); ++i)
  {
    // Sort all sets of edge nodes (needed for comparing edges)
    std::sort(edge_nodes[i].begin(), edge_nodes[i].end());

    std::vector<PenetrationInfo *> face_info_comm_edge;
    getInfoForFacesWithCommonNodes(
        secondary_node, elems_to_exclude, edge_nodes[i], face_info_comm_edge, p_info);

    if (face_info_comm_edge.size() == 0)
      edges_without_neighbors.push_back(i);
    else if (face_info_comm_edge.size() > 1)
      mooseError("Only one neighbor allowed per edge");
    else
      edge_face_info[i] = face_info_comm_edge[0];
  }

  // Remove edges without neighbors from the vector, starting from end
  std::vector<unsigned int>::reverse_iterator rit;
  for (rit = edges_without_neighbors.rbegin(); rit != edges_without_neighbors.rend(); ++rit)
  {
    unsigned int index = *rit;
    edge_nodes.erase(edge_nodes.begin() + index);
    edge_face_weights.erase(edge_face_weights.begin() + index);
    edge_face_info.erase(edge_face_info.begin() + index);
  }

  // Handle corner case
  if (edge_nodes.size() > 1)
  {
    if (edge_nodes.size() != 2)
      mooseError("Invalid number of smoothing edges");

    // find common node
    std::vector<const Node *> common_nodes;
    std::set_intersection(edge_nodes[0].begin(),
                          edge_nodes[0].end(),
                          edge_nodes[1].begin(),
                          edge_nodes[1].end(),
                          std::inserter(common_nodes, common_nodes.end()));

    if (common_nodes.size() != 1)
      mooseError("Invalid number of common nodes");

    for (const auto & pinfo : edge_face_info)
      elems_to_exclude.insert(pinfo->_elem->id());

    std::vector<PenetrationInfo *> face_info_comm_edge;
    getInfoForFacesWithCommonNodes(
        secondary_node, elems_to_exclude, common_nodes, face_info_comm_edge, p_info);

    unsigned int num_corner_neighbors = face_info_comm_edge.size();

    if (num_corner_neighbors > 0)
    {
      Real fw0 = edge_face_weights[0];
      Real fw1 = edge_face_weights[1];

      // Corner weight is product of edge weights.  Spread out over multiple neighbors.
      Real fw_corner = (fw0 * fw1) / static_cast<Real>(num_corner_neighbors);

      // Adjust original edge weights
      edge_face_weights[0] *= (1.0 - fw1);
      edge_face_weights[1] *= (1.0 - fw0);

      for (unsigned int i = 0; i < num_corner_neighbors; ++i)
      {
        edge_face_weights.push_back(fw_corner);
        edge_face_info.push_back(face_info_comm_edge[i]);
      }
    }
  }
}

void
PenetrationThread::getSmoothingEdgeNodesAndWeights(
    const Point & p,
    const Elem * side,
    std::vector<std::vector<const Node *>> & edge_nodes,
    std::vector<Real> & edge_face_weights)
{
  const ElemType t(side->type());
  const Real & xi = p(0);
  const Real & eta = p(1);

  Real smooth_limit = 1.0 - _normal_smoothing_distance;

  switch (t)
  {
    case EDGE2:
    case EDGE3:
    case EDGE4:
    {
      if (xi < -smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(0));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 + xi) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      else if (xi > smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(1));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 - xi) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      break;
    }

    case TRI3:
    case TRI6:
    {
      if (eta < -smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(0));
        en.push_back(side->node_ptr(1));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 + eta) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      if ((xi + eta) > smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(1));
        en.push_back(side->node_ptr(2));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 - xi - eta) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      if (xi < -smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(2));
        en.push_back(side->node_ptr(0));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 + xi) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      break;
    }

    case QUAD4:
    case QUAD8:
    case QUAD9:
    {
      if (eta < -smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(0));
        en.push_back(side->node_ptr(1));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 + eta) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      if (xi > smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(1));
        en.push_back(side->node_ptr(2));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 - xi) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      if (eta > smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(2));
        en.push_back(side->node_ptr(3));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 - eta) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
      }
      if (xi < -smooth_limit)
      {
        std::vector<const Node *> en;
        en.push_back(side->node_ptr(3));
        en.push_back(side->node_ptr(0));
        edge_nodes.push_back(en);
        Real fw = 0.5 - (1.0 + xi) / (2.0 * _normal_smoothing_distance);
        if (fw > 0.5)
          fw = 0.5;
        edge_face_weights.push_back(fw);
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

void
PenetrationThread::getInfoForFacesWithCommonNodes(
    const Node * secondary_node,
    const std::set<dof_id_type> & elems_to_exclude,
    const std::vector<const Node *> edge_nodes,
    std::vector<PenetrationInfo *> & face_info_comm_edge,
    std::vector<PenetrationInfo *> & p_info)
{
  // elems connected to a node on this edge, find one that has the same corners as this, and is not
  // the current elem
  auto node_to_elem_pair = _node_to_elem_map.find(edge_nodes[0]->id()); // just need one of the
                                                                        // nodes
  mooseAssert(node_to_elem_pair != _node_to_elem_map.end(), "Missing entry in node to elem map");
  const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;

  std::vector<const Elem *> elems_connected_to_edge;

  for (unsigned int ecni = 0; ecni < elems_connected_to_node.size(); ecni++)
  {
    if (elems_to_exclude.find(elems_connected_to_node[ecni]) != elems_to_exclude.end())
      continue;
    const Elem * elem = _mesh.elemPtr(elems_connected_to_node[ecni]);

    std::vector<const Node *> nodevec;
    for (unsigned int ni = 0; ni < elem->n_nodes(); ++ni)
      if (elem->is_vertex(ni))
        nodevec.push_back(elem->node_ptr(ni));

    std::vector<const Node *> common_nodes;
    std::sort(nodevec.begin(), nodevec.end());
    std::set_intersection(edge_nodes.begin(),
                          edge_nodes.end(),
                          nodevec.begin(),
                          nodevec.end(),
                          std::inserter(common_nodes, common_nodes.end()));

    if (common_nodes.size() == edge_nodes.size())
      elems_connected_to_edge.push_back(elem);
  }

  if (elems_connected_to_edge.size() > 0)
  {

    // There are potentially multiple elements that share a common edge
    // 2D:
    // There can only be one element on the same surface
    // 3D:
    // If there are two edge nodes, there can only be one element on the same surface
    // If there is only one edge node (a corner), there could be multiple elements on the same
    // surface
    bool allowMultipleNeighbors = false;

    if (elems_connected_to_edge[0]->dim() == 3)
    {
      if (edge_nodes.size() == 1)
      {
        allowMultipleNeighbors = true;
      }
    }

    for (unsigned int i = 0; i < elems_connected_to_edge.size(); ++i)
    {
      std::vector<PenetrationInfo *> thisElemInfo;
      getInfoForElem(thisElemInfo, p_info, elems_connected_to_edge[i]);
      if (thisElemInfo.size() > 0 && !allowMultipleNeighbors)
      {
        if (thisElemInfo.size() > 1)
          mooseError(
              "Found multiple neighbors to current edge/face on surface when only one is allowed");
        face_info_comm_edge.push_back(thisElemInfo[0]);
        break;
      }

      createInfoForElem(
          thisElemInfo, p_info, secondary_node, elems_connected_to_edge[i], edge_nodes);
      if (thisElemInfo.size() > 0 && !allowMultipleNeighbors)
      {
        if (thisElemInfo.size() > 1)
          mooseError(
              "Found multiple neighbors to current edge/face on surface when only one is allowed");
        face_info_comm_edge.push_back(thisElemInfo[0]);
        break;
      }

      for (unsigned int j = 0; j < thisElemInfo.size(); ++j)
        face_info_comm_edge.push_back(thisElemInfo[j]);
    }
  }
}

void
PenetrationThread::getInfoForElem(std::vector<PenetrationInfo *> & thisElemInfo,
                                  std::vector<PenetrationInfo *> & p_info,
                                  const Elem * elem)
{
  for (const auto & pi : p_info)
  {
    if (!pi)
      continue;

    if (pi->_elem == elem)
      thisElemInfo.push_back(pi);
  }
}

void
PenetrationThread::createInfoForElem(std::vector<PenetrationInfo *> & thisElemInfo,
                                     std::vector<PenetrationInfo *> & p_info,
                                     const Node * secondary_node,
                                     const Elem * elem,
                                     const std::vector<const Node *> & nodes_that_must_be_on_side,
                                     const bool check_whether_reasonable)
{
  std::vector<unsigned int> sides;
  // TODO: After libMesh update, add this line to MooseMesh.h, call sidesWithBoundaryID,  delete
  // getSidesOnPrimaryBoundary, and delete vectors used by it
  //  void sidesWithBoundaryID(std::vector<unsigned int>& sides, const Elem * const elem, const
  //  BoundaryID boundary_id) const
  // {
  //   _mesh.get_boundary_info().sides_with_boundary_id(sides, elem, boundary_id);
  // }
  getSidesOnPrimaryBoundary(sides, elem);
  // _mesh.sidesWithBoundaryID(sides, elem, _primary_boundary);

  for (unsigned int i = 0; i < sides.size(); ++i)
  {
    // Don't create info for this side if one already exists
    bool already_have_info_this_side = false;
    for (const auto & pi : thisElemInfo)
      if (pi->_side_num == sides[i])
      {
        already_have_info_this_side = true;
        break;
      }

    if (already_have_info_this_side)
      break;

    const Elem * side = (elem->build_side_ptr(sides[i], false)).release();

    // Only continue with creating info for this side if the side contains
    // all of the nodes in nodes_that_must_be_on_side
    std::vector<const Node *> nodevec;
    for (unsigned int ni = 0; ni < side->n_nodes(); ++ni)
      nodevec.push_back(side->node_ptr(ni));

    std::sort(nodevec.begin(), nodevec.end());
    std::vector<const Node *> common_nodes;
    std::set_intersection(nodes_that_must_be_on_side.begin(),
                          nodes_that_must_be_on_side.end(),
                          nodevec.begin(),
                          nodevec.end(),
                          std::inserter(common_nodes, common_nodes.end()));
    if (common_nodes.size() != nodes_that_must_be_on_side.size())
    {
      delete side;
      break;
    }

    FEBase * fe_elem = _fes[_tid][elem->dim()];
    FEBase * fe_side = _fes[_tid][side->dim()];

    // Prerequest the data we'll need in findContactPoint
    fe_side->get_phi();
    fe_side->get_dphi();
    fe_side->get_xyz();
    fe_side->get_dxyzdxi();
    fe_side->get_d2xyzdxi2();
    fe_side->get_d2xyzdxideta();
    fe_side->get_dxyzdeta();
    fe_side->get_d2xyzdeta2();
    fe_side->get_d2xyzdxideta();

    // Optionally check to see whether face is reasonable candidate based on an
    // estimate of how closely it is likely to project to the face
    if (check_whether_reasonable)
      if (!isFaceReasonableCandidate(elem, side, fe_side, secondary_node, _tangential_tolerance))
      {
        delete side;
        break;
      }

    Point contact_phys;
    Point contact_ref;
    Point contact_on_face_ref;
    Real distance = 0.;
    Real tangential_distance = 0.;
    RealGradient normal;
    bool contact_point_on_side;
    std::vector<const Node *> off_edge_nodes;
    std::vector<std::vector<Real>> side_phi;
    std::vector<std::vector<RealGradient>> side_grad_phi;
    std::vector<RealGradient> dxyzdxi;
    std::vector<RealGradient> dxyzdeta;
    std::vector<RealGradient> d2xyzdxideta;

    PenetrationInfo * pen_info = new PenetrationInfo(secondary_node,
                                                     elem,
                                                     side,
                                                     sides[i],
                                                     normal,
                                                     distance,
                                                     tangential_distance,
                                                     contact_phys,
                                                     contact_ref,
                                                     contact_on_face_ref,
                                                     off_edge_nodes,
                                                     side_phi,
                                                     side_grad_phi,
                                                     dxyzdxi,
                                                     dxyzdeta,
                                                     d2xyzdxideta);

    Moose::findContactPoint(*pen_info,
                            fe_elem,
                            fe_side,
                            _fe_type,
                            *secondary_node,
                            true,
                            _tangential_tolerance,
                            contact_point_on_side);

    thisElemInfo.push_back(pen_info);

    p_info.push_back(pen_info);
  }
}

// TODO: After libMesh update, replace this with a call to sidesWithBoundaryID, delete vectors used
// by this method
void
PenetrationThread::getSidesOnPrimaryBoundary(std::vector<unsigned int> & sides,
                                             const Elem * const elem)
{
  // For each tuple, the fields are (0=elem_id, 1=side_id, 2=bc_id)
  sides.clear();
  struct Comp
  {
    bool operator()(const libMesh::BoundaryInfo::BCTuple & tup, dof_id_type id) const
    {
      return std::get<0>(tup) < id;
    }
    bool operator()(dof_id_type id, const libMesh::BoundaryInfo::BCTuple & tup) const
    {
      return id < std::get<0>(tup);
    }
  };

  auto range = std::equal_range(_bc_tuples.begin(), _bc_tuples.end(), elem->id(), Comp{});

  for (auto & t = range.first; t != range.second; ++t)
    if (std::get<2>(*t) == static_cast<boundary_id_type>(_primary_boundary))
      sides.push_back(std::get<1>(*t));
}
