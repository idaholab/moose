//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAElement2D.h"

#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFace.h"
#include "EFAFragment2D.h"

#include "EFAFaceNode.h"
#include "EFAFuncs.h"
#include "EFAError.h"
#include "XFEMFuncs.h"

EFAFragment2D::EFAFragment2D(EFAElement2D * host,
                             bool create_boundary_edges,
                             const EFAElement2D * from_host,
                             unsigned int frag_id)
  : EFAFragment(), _host_elem(host)
{
  if (create_boundary_edges)
  {
    if (!from_host)
      EFAError("EFAfragment2D constructor must have a from_host to copy from");
    if (frag_id == std::numeric_limits<unsigned int>::max()) // copy the from_host itself
    {
      for (unsigned int i = 0; i < from_host->numEdges(); ++i)
        _boundary_edges.push_back(new EFAEdge(*from_host->getEdge(i)));
    }
    else
    {
      if (frag_id > from_host->numFragments() - 1)
        EFAError("In EFAfragment2D constructor fragment_copy_index out of bounds");
      for (unsigned int i = 0; i < from_host->getFragment(frag_id)->numEdges(); ++i)
        _boundary_edges.push_back(new EFAEdge(*from_host->getFragmentEdge(frag_id, i)));
    }
  }
}

EFAFragment2D::EFAFragment2D(EFAElement2D * host, const EFAFace * from_face)
  : EFAFragment(), _host_elem(host)
{
  for (unsigned int i = 0; i < from_face->numEdges(); ++i)
    _boundary_edges.push_back(new EFAEdge(*from_face->getEdge(i)));
}

EFAFragment2D::~EFAFragment2D()
{
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (_boundary_edges[i])
    {
      delete _boundary_edges[i];
      _boundary_edges[i] = NULL;
    }
  }
}

void
EFAFragment2D::switchNode(EFANode * new_node, EFANode * old_node)
{
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    _boundary_edges[i]->switchNode(new_node, old_node);
}

bool
EFAFragment2D::containsNode(EFANode * node) const
{
  bool contains = false;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (_boundary_edges[i]->containsNode(node))
    {
      contains = true;
      break;
    }
  }
  return contains;
}

unsigned int
EFAFragment2D::getNumCuts() const
{
  unsigned int num_cut_edges = 0;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (_boundary_edges[i]->hasIntersection())
      num_cut_edges += _boundary_edges[i]->numEmbeddedNodes();
  }
  return num_cut_edges;
}

unsigned int
EFAFragment2D::getNumCutNodes() const
{
  unsigned int num_cut_nodes = 0;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    if (_boundary_edges[i]->getNode(0)->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
      num_cut_nodes++;
  return num_cut_nodes;
}

std::set<EFANode *>
EFAFragment2D::getAllNodes() const
{
  std::set<EFANode *> nodes;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    nodes.insert(_boundary_edges[i]->getNode(0));
    nodes.insert(_boundary_edges[i]->getNode(1));
  }
  return nodes;
}

bool
EFAFragment2D::isConnected(EFAFragment * other_fragment) const
{
  bool is_connected = false;
  EFAFragment2D * other_frag2d = dynamic_cast<EFAFragment2D *>(other_fragment);
  if (!other_frag2d)
    EFAError("in isConnected other_fragment is not of type EFAfragement2D");

  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    for (unsigned int j = 0; j < other_frag2d->numEdges(); ++j)
    {
      if (_boundary_edges[i]->equivalent(*other_frag2d->getEdge(j)))
      {
        is_connected = true;
        break;
      }
    }
    if (is_connected)
      break;
  } // i
  return is_connected;
}

void
EFAFragment2D::removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes)
{
  // if a fragment only has 1 intersection which is in an interior edge
  // remove this embedded node (MUST DO THIS AFTER combine_tip_edges())
  if (getNumCuts() == 1)
  {
    for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    {
      if (isEdgeInterior(i) && _boundary_edges[i]->hasIntersection())
      {
        if (_host_elem->numInteriorNodes() != 1)
          EFAError("host element must have 1 interior node at this point");
        Efa::deleteFromMap(EmbeddedNodes, _boundary_edges[i]->getEmbeddedNode(0));
        _boundary_edges[i]->removeEmbeddedNodes();
        _host_elem->deleteInteriorNodes();
        break;
      }
    } // i
  }
}

void
EFAFragment2D::combineTipEdges()
{
  // combine the tip edges in a crack tip fragment
  // N.B. the host elem can only have one elem_tip_edge, otherwise it should have already been
  // completely split
  if (!_host_elem)
    EFAError("In combine_tip_edges() the frag must have host_elem");

  bool has_tip_edges = false;
  unsigned int elem_tip_edge_id = std::numeric_limits<unsigned int>::max();
  std::vector<unsigned int> frag_tip_edge_id;
  for (unsigned int i = 0; i < _host_elem->numEdges(); ++i)
  {
    frag_tip_edge_id.clear();
    if (_host_elem->getEdge(i)->hasIntersection())
    {
      for (unsigned int j = 0; j < _boundary_edges.size(); ++j)
      {
        if (_host_elem->getEdge(i)->containsEdge(*_boundary_edges[j]))
          frag_tip_edge_id.push_back(j);
      }                                 // j
      if (frag_tip_edge_id.size() == 2) // combine the two frag edges on this elem edge
      {
        has_tip_edges = true;
        elem_tip_edge_id = i;
        break;
      }
    }
  } // i
  if (has_tip_edges)
  {
    // frag_tip_edge_id[0] must precede frag_tip_edge_id[1]
    unsigned int edge0_next(frag_tip_edge_id[0] < (numEdges() - 1) ? frag_tip_edge_id[0] + 1 : 0);
    if (edge0_next != frag_tip_edge_id[1])
      EFAError("frag_tip_edge_id[1] must be the next edge of frag_tip_edge_id[0]");

    // get the two end nodes of the new edge
    EFANode * node1 = _boundary_edges[frag_tip_edge_id[0]]->getNode(0);
    EFANode * emb_node = _boundary_edges[frag_tip_edge_id[0]]->getNode(1);
    EFANode * node2 = _boundary_edges[frag_tip_edge_id[1]]->getNode(1);
    if (emb_node != _boundary_edges[frag_tip_edge_id[1]]->getNode(0))
      EFAError("fragment edges are not correctly set up");

    // get the new edge with one intersection
    EFAEdge * elem_edge = _host_elem->getEdge(elem_tip_edge_id);
    double xi_node1 = elem_edge->distanceFromNode1(node1);
    double xi_node2 = elem_edge->distanceFromNode1(node2);
    double xi_emb = elem_edge->distanceFromNode1(emb_node);
    double position = (xi_emb - xi_node1) / (xi_node2 - xi_node1);
    EFAEdge * full_edge = new EFAEdge(node1, node2);
    full_edge->addIntersection(position, emb_node, node1);
    _host_elem->addNodeToCutPlaneIdx(emb_node, _host_elem->getCurrentCutPlaneIdx());
    // TODO::add cut plane id here for combineTipEdges

    // combine the two original fragment edges
    delete _boundary_edges[frag_tip_edge_id[0]];
    delete _boundary_edges[frag_tip_edge_id[1]];
    _boundary_edges[frag_tip_edge_id[0]] = full_edge;
    _boundary_edges.erase(_boundary_edges.begin() + frag_tip_edge_id[1]);
  }
}

/*
std::vector<EFAnode*>
EFAfragment::commonNodesWithEdge(EFAEdge & other_edge)
{
  std::vector<EFAnode*> common_nodes;
  for (unsigned int i = 0; i < 2; ++i)
  {
    EFAnode* edge_node = other_edge.node_ptr(i);
    if (containsNode(edge_node))
      common_nodes.push_back(edge_node);
  }
  return common_nodes;
}
*/

bool
EFAFragment2D::isEdgeInterior(unsigned int edge_id) const
{
  if (!_host_elem)
    EFAError("in isEdgeInterior fragment must have host elem");

  bool edge_in_elem_edge = false;

  for (unsigned int i = 0; i < _host_elem->numEdges(); ++i)
  {
    if (_host_elem->getEdge(i)->containsEdge(*_boundary_edges[edge_id]))
    {
      edge_in_elem_edge = true;
      break;
    }
  }
  if (!edge_in_elem_edge)
    return true; // yes, is interior
  else
    return false;
}

std::vector<unsigned int>
EFAFragment2D::getInteriorEdgeID() const
{
  std::vector<unsigned int> interior_edge_id;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (isEdgeInterior(i))
      interior_edge_id.push_back(i);
  }
  return interior_edge_id;
}

bool
EFAFragment2D::isSecondaryInteriorEdge(unsigned int edge_id) const
{
  bool is_second_cut = false;
  if (!_host_elem)
    EFAError("in isSecondaryInteriorEdge fragment must have host elem");

  for (unsigned int i = 0; i < _host_elem->numInteriorNodes(); ++i)
  {
    if (_boundary_edges[edge_id]->containsNode(_host_elem->getInteriorNode(i)->getNode()))
    {
      is_second_cut = true;
      break;
    }
  }
  return is_second_cut;
}

unsigned int
EFAFragment2D::numEdges() const
{
  return _boundary_edges.size();
}

EFAEdge *
EFAFragment2D::getEdge(unsigned int edge_id) const
{
  if (edge_id > _boundary_edges.size() - 1)
    EFAError("in EFAfragment2D::get_edge, index out of bounds");
  return _boundary_edges[edge_id];
}

void
EFAFragment2D::addEdge(EFAEdge * new_edge)
{
  _boundary_edges.push_back(new_edge);
}

std::set<EFANode *>
EFAFragment2D::getEdgeNodes(unsigned int edge_id) const
{
  std::set<EFANode *> edge_nodes;
  edge_nodes.insert(_boundary_edges[edge_id]->getNode(0));
  edge_nodes.insert(_boundary_edges[edge_id]->getNode(1));
  return edge_nodes;
}

EFAElement2D *
EFAFragment2D::getHostElement() const
{
  return _host_elem;
}

std::vector<EFAFragment2D *>
EFAFragment2D::split(std::map<unsigned int, EFANode *> & EmbeddedNodes)
{
  return split(EmbeddedNodes, _host_elem->getCutPlaneIndices(), _host_elem->getCutPlaneNodes());
}

std::vector<EFAFragment2D *>
EFAFragment2D::split(std::map<unsigned int, EFANode *> & EmbeddedNodes,
                     std::vector<unsigned int> frag_cut_plane_idx,
                     std::vector<std::vector<EFANode *>> frag_cut_plane_nodes)
{
  // For 2D only not working in 3D
  // This method will split one existing fragment into multiple
  // new fragments and return them.
  std::vector<EFAFragment2D *> new_fragments;
  std::vector<std::vector<EFANode *>> fragment_nodes(
      2);                       // vectors of EFA nodes in the two fragments
  unsigned int frag_number = 0; // Index of the current fragment that we are assembling nodes into
  unsigned int edge_cut_count = 0;
  unsigned int node_cut_count = 0;

  std::vector<unsigned int> all_cut_plane_idx;
  all_cut_plane_idx = frag_cut_plane_idx;

  std::vector<std::vector<EFANode *>> all_cut_plane_nodes;
  all_cut_plane_nodes = frag_cut_plane_nodes;

  unsigned int cut_plane_id;
  std::vector<EFANode *> chosen_cut_plane_pair;
  std::vector<std::vector<EFANode *>>
      cut_plane_node_pairs; // vector of node pairs for each identified cut plane including the cut
                            // plane we are splitting on first
  std::vector<std::vector<double>> chosen_cut_plane_pair_points;
  std::vector<std::vector<std::vector<double>>> cut_plane_node_pair_points;
  std::vector<bool> split_frag = {false, false}; // tag for recursive split on frags
  std::vector<EFAFaceNode *> allIntersectionFaceNodes;
  unsigned int frag0_first_cut_pos; // needed to tag location to insert new intersection nodes frag1
                                    // is always the final position
  bool frag0_first_cut_pos_found = false;

  if (all_cut_plane_idx.size() > 0) // if there are no cut planes for this element do nothing
  {
    cut_plane_id = all_cut_plane_idx.back(); // chosen cut plane id
    all_cut_plane_idx.pop_back();

    chosen_cut_plane_pair = all_cut_plane_nodes.back(); // chosen cut plane node(s)
    all_cut_plane_nodes.pop_back();

    cut_plane_node_pairs = all_cut_plane_nodes;

    // start looking through nodes
    for (unsigned int iedge = 0; iedge < _boundary_edges.size(); ++iedge)
    {
      fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getNode(0));

      bool frag_switch = false;
      if (_host_elem->hasSameCut(_boundary_edges[iedge]->getNode(0),
                                 cut_plane_id)) // if it is one of the chosen nodes
      {
        if (!frag0_first_cut_pos_found)
        {
          frag0_first_cut_pos =
              fragment_nodes[0].size(); // record position for insertion of intersection nodes
          frag0_first_cut_pos_found = true;
        }
        frag_switch = true;
        ++node_cut_count;
        frag_number = 1 - frag_number; // Toggle between 0 and 1
        fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getNode(0));
      }
      // note the cut plane ids from nodes to determine if fragments need split again
      bool moreCuts = false;
      for (unsigned int cut = 0; cut < all_cut_plane_idx.size(); ++cut)
      {
        if (_host_elem->hasSameCut(_boundary_edges[iedge]->getNode(0), all_cut_plane_idx[cut]))
        {
          moreCuts = true;
          break;
        }
      }

      if (moreCuts) // note the current frag has more cuts so needs to be split again
      {
        split_frag[frag_number] = true;
        if (frag_switch)
          split_frag[1 - frag_number] = true;
      }

      // order embedded nodes
      std::vector<EFANode *> orderedEmbeddedNodes = {};
      if (_boundary_edges[iedge]->numEmbeddedNodes() > 0)
      {
        orderedEmbeddedNodes.push_back(_boundary_edges[iedge]->getEmbeddedNode(0));
        if (_boundary_edges[iedge]->numEmbeddedNodes() > 1)
        {
          std::vector<double> node0Position = {};
          _host_elem->getNodeParametricCoordinate(_boundary_edges[iedge]->getNode(0),
                                                  node0Position);
          for (unsigned int iembeddedNode = 1;
               iembeddedNode < _boundary_edges[iedge]->numEmbeddedNodes();
               ++iembeddedNode)
          {
            std::vector<double> iNodePosition = {};
            _host_elem->getNodeParametricCoordinate(
                _boundary_edges[iedge]->getEmbeddedNode(iembeddedNode), iNodePosition);
            for (unsigned int position = 0; position < orderedEmbeddedNodes.size(); ++position)
            {
              // insert edge nodes based on distance from edge node [0] so it
              // is easy to add them correctly to each fragment
              std::vector<double> current_node_pos;
              _host_elem->getNodeParametricCoordinate(orderedEmbeddedNodes[iembeddedNode],
                                                      current_node_pos);
              if (distanceBetweenPoints(iNodePosition, node0Position) <
                  distanceBetweenPoints(current_node_pos, node0Position))
              {
                orderedEmbeddedNodes.insert(orderedEmbeddedNodes.begin() + position,
                                            _boundary_edges[iedge]->getEmbeddedNode(iembeddedNode));
                break;
              }
            }
          }
        }
      }

      // check embedded nodes along the current boundary edge
      for (unsigned int i = 0; i < _boundary_edges[iedge]->numEmbeddedNodes();
           ++i) // this loop only happens on embedded nodes which are cuts
      {
        bool frag_switch = false;
        fragment_nodes[frag_number].push_back(orderedEmbeddedNodes[i]);

        if (_host_elem->hasSameCut(orderedEmbeddedNodes[i], cut_plane_id))
        {
          if (!frag0_first_cut_pos_found)
          {
            frag0_first_cut_pos = fragment_nodes[0].size(); // record position for intersection
                                                            // nodes
            frag0_first_cut_pos_found = true;
          }
          frag_switch = true;
          ++edge_cut_count;
          frag_number = 1 - frag_number; // Toggle between 0 and 1
          fragment_nodes[frag_number].push_back(orderedEmbeddedNodes[i]);
        }
        // note the cut plane ids from nodes to determine if fragments need split again
        bool moreCuts = false;
        for (unsigned int cut = 0; cut < all_cut_plane_idx.size(); ++cut)
        {
          if (_host_elem->hasSameCut(orderedEmbeddedNodes[i], cut))
          {
            moreCuts = true;
            break;
          }
        }

        if (moreCuts) // note the current frag has more cuts so needs to be split again
        {
          split_frag[frag_number] = true;
          if (frag_switch)
            split_frag[1 - frag_number] = true;
        }
      }
    }

    // remove chosen cut nodes from host if they were found
    if ((edge_cut_count + node_cut_count) == chosen_cut_plane_pair.size())
    {
      for (unsigned int i = 0; i < chosen_cut_plane_pair.size(); ++i)
        _host_elem->removeCutPlaneNode(chosen_cut_plane_pair[i], cut_plane_id);
    }

    // check other cut planes for pairs before making intersection lines
    bool other_pairs = false;
    for (unsigned int ipair = 0; ipair < cut_plane_node_pairs.size(); ++ipair)
      if (cut_plane_node_pairs[ipair].size() == 2)
        other_pairs = true;

    // add secondary interior embedded nodes here with intersectsegmentwithcutline function against
    // the chosen cut plane first get chosen cut node locations
    if (chosen_cut_plane_pair.size() == 2 &&
        other_pairs) // if the chosen cut plane id is only one node don't check for interior
                     // intersections
    {
      std::vector<double> para_point_coor;
      for (unsigned int node = 0; node < chosen_cut_plane_pair.size();
           ++node) // set chosen cutting pair coordinates
      {
        if (_host_elem->getNodeParametricCoordinate(chosen_cut_plane_pair[node], para_point_coor))
        {
          chosen_cut_plane_pair_points.push_back(para_point_coor);
        }
        else // Error EFANode not on edge
          EFAError("EFANode is not a Face Node and not on any edge");
      }
      // now find the other node pair points
      cut_plane_node_pair_points.resize(cut_plane_node_pairs.size());
      for (unsigned int pair = 0; pair < cut_plane_node_pairs.size(); ++pair)
      {
        if (cut_plane_node_pairs[pair].size() == 2)
          for (unsigned int node = 0; node < 2; ++node) // for each node in the pair
          {
            para_point_coor = {-999, -999};
            if (_host_elem->getNodeParametricCoordinate(cut_plane_node_pairs[pair][node],
                                                        para_point_coor))
            {
              cut_plane_node_pair_points[pair].push_back(para_point_coor);
            }
            else // Error EFANode not on edge
              EFAError("EFANode is not a Face Node and not on any edge");
          }
      }
      // now determine if there are intersections between those points
      std::vector<double> intersection_point = {-999, -999};
      for (unsigned int id = 0; id < cut_plane_node_pair_points.size(); ++id)
      {
        // see if current node pair intersect the chosen cut plane node pair
        intersection_point = {-999, -999};
        if (cut_plane_node_pair_points[id].size() == 2)
          if (IntersectSegmentWithCutLine(cut_plane_node_pair_points[id][0],
                                          cut_plane_node_pair_points[id][1],
                                          chosen_cut_plane_pair_points[0],
                                          chosen_cut_plane_pair_points[1],
                                          intersection_point))
          {
            unsigned int new_node_id = Efa::getNewID(EmbeddedNodes);
            EFANode * embedded_node = new EFANode(new_node_id, EFANode::N_CATEGORY_EMBEDDED);
            EmbeddedNodes.insert(std::make_pair(new_node_id, embedded_node));

            // add node to cut planes since it now had the intersecting cut plane id
            // must reassign second intersecting node to new cut plane to make each a new pair with
            // the intersection point
            _host_elem->getNewCutPlaneIdx(); // new cut plane for new pair
            _host_elem->reassignNodeToCutPlaneIdx(
                cut_plane_node_pairs[id][1],
                _host_elem->getCurrentCutPlaneIdx()); // separate the old pair
            _host_elem->addNodeToCutPlaneIdx(
                embedded_node,
                all_cut_plane_idx[id]); // make pair with embedded node and node 0 is old cut plane
            _host_elem->addNodeToCutPlaneIdx(
                embedded_node, _host_elem->getCurrentCutPlaneIdx()); // make pair with embedded node
                                                                     // and node 1 in new cut plane
            //            _host_elem->addForceNodeToCutPlaneIdx(embedded_node,
            //            _host_elem->getCurrentCutPlaneIdx());//add extra listing with embedded
            //            node in new cut plane since it could be depleted by the other fragment
            //            checking it first

            all_cut_plane_idx = _host_elem->getCutPlaneIndices();
            all_cut_plane_nodes = _host_elem->getCutPlaneNodes();

            EFAFaceNode * intersectionFaceNode =
                new EFAFaceNode(embedded_node, intersection_point[0], intersection_point[1]);
            _host_elem->addInteriorNode(intersectionFaceNode);
            if (allIntersectionFaceNodes.size() == 0)
              allIntersectionFaceNodes.push_back(intersectionFaceNode);
            else
            {
              for (unsigned int position = 0; position < allIntersectionFaceNodes.size();
                   ++position)
              {
                // insert intersection nodes based on distance from chosen cut plane node [0] so it
                // is easy to add them correctly to each fragment
                std::vector<double> current_node_pos = {
                    allIntersectionFaceNodes[position]->getParametricCoordinates(0),
                    allIntersectionFaceNodes[position]->getParametricCoordinates(1)};
                if (distanceBetweenPoints(intersection_point, chosen_cut_plane_pair_points[0]) <
                    distanceBetweenPoints(current_node_pos, chosen_cut_plane_pair_points[0]))
                {
                  allIntersectionFaceNodes.insert(allIntersectionFaceNodes.begin() + position,
                                                  intersectionFaceNode);
                  break;
                }
              }
            }
          }
      }
      // add new intersection nodes to each fragment
      if (allIntersectionFaceNodes.size() != 0)
      {
        // frag0 add intersection nodes at marked location using frag0_first_cut_pos
        for (signed int int_id = allIntersectionFaceNodes.size() - 1; int_id >= 0; --int_id)
          fragment_nodes[0].insert(fragment_nodes[0].begin() + frag0_first_cut_pos,
                                   allIntersectionFaceNodes[int_id]->getNode());
        // frag1 add  intersection nodes at the end of the vector
        for (signed int int_id = allIntersectionFaceNodes.size() - 1; int_id >= 0; --int_id)
          fragment_nodes[1].push_back(allIntersectionFaceNodes[int_id]->getNode());
      }
    }
  }

  std::vector<EFAFragment2D *> final_fragments; // final fragments for return

  // only make new fragments if all the chosen nodes were found
  if ((edge_cut_count + node_cut_count) == chosen_cut_plane_pair.size())
  {
    // add nodes to fragments
    if ((edge_cut_count + node_cut_count) > 1) // any two cuts case
    {
      for (unsigned int frag_idx = 0; frag_idx < 2; ++frag_idx) // Create 2 fragments
      {
        auto & this_frag_nodes = fragment_nodes[frag_idx];
        // check to make sure an edge wasn't cut
        if (this_frag_nodes.size() >= 3)
        {
          EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
          for (unsigned int inode = 0; inode < this_frag_nodes.size() - 1; inode++)
            new_frag->addEdge(new EFAEdge(this_frag_nodes[inode], this_frag_nodes[inode + 1]));

          new_frag->addEdge(
              new EFAEdge(this_frag_nodes[this_frag_nodes.size() - 1], this_frag_nodes[0]));

          new_fragments.push_back(new_frag);
        }
      }
    }
    else if (edge_cut_count == 1) // single edge cut case
    {
      EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
      for (unsigned int inode = 0; inode < fragment_nodes[0].size() - 1;
           inode++) // assemble fragment part 1
        new_frag->addEdge(new EFAEdge(fragment_nodes[0][inode], fragment_nodes[0][inode + 1]));

      for (unsigned int inode = 0; inode < fragment_nodes[1].size() - 1;
           inode++) // assemble fragment part 2
        new_frag->addEdge(new EFAEdge(fragment_nodes[1][inode], fragment_nodes[1][inode + 1]));

      new_frag->addEdge(
          new EFAEdge(fragment_nodes[1][fragment_nodes[1].size() - 1], fragment_nodes[0][0]));

      new_fragments.push_back(new_frag);
    }
    else if (node_cut_count == 1) // single node cut case
    {
      EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
      for (unsigned int iedge = 0; iedge < _boundary_edges.size(); ++iedge)
      {
        EFANode * first_node_on_edge = _boundary_edges[iedge]->getNode(0);
        EFANode * second_node_on_edge = _boundary_edges[iedge]->getNode(1);
        new_frag->addEdge(new EFAEdge(first_node_on_edge, second_node_on_edge));
      }

      new_fragments.push_back(new_frag);
    }
  }
  else // if the correct number of chosen nodes were not found call again lookin for other nodes
  {
    return this->split(EmbeddedNodes, all_cut_plane_idx, all_cut_plane_nodes);
  }

  for (unsigned int frag_idx = 0; frag_idx < new_fragments.size();
       ++frag_idx) // see if new fragments need split again
  {
    if (split_frag[frag_idx])
    {
      std::vector<EFAFragment2D *> temp_frags;
      temp_frags =
          new_fragments[frag_idx]->split(EmbeddedNodes, all_cut_plane_idx, all_cut_plane_nodes);
      if (temp_frags.size() == 0)
        final_fragments.push_back(new_fragments[frag_idx]);
      else
        for (unsigned int ifrag = 0; ifrag < temp_frags.size(); ++ifrag)
          final_fragments.push_back(temp_frags[ifrag]);
    }
    else
      final_fragments.push_back(new_fragments[frag_idx]);
  }

  return final_fragments;
}

bool
EFAFragment2D::IntersectSegmentWithCutLine(const std::vector<double> & segment_point1,
                                           const std::vector<double> & segment_point2,
                                           const std::vector<double> & cutting_line_point1,
                                           const std::vector<double> & cutting_line_point2,
                                           std::vector<double> & intersect_Point)
{
  // Use the algorithm described here to determine whether a line segment is intersected
  // by a cutting line, and to compute the fraction along that line where the intersection
  // occurs:
  // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  // additionally modified for the specific use here
  intersect_Point.resize(2, 0.0);
  bool cut_segment = false;
  std::vector<double> seg_dir = {0, 0};
  seg_dir[0] = segment_point2[0] - segment_point1[0];
  seg_dir[1] = segment_point2[1] - segment_point1[1];
  std::vector<double> cut_dir = {0, 0};
  cut_dir[0] = cutting_line_point2[0] - cutting_line_point1[0];
  cut_dir[1] = cutting_line_point2[1] - cutting_line_point1[1];
  std::vector<double> cut_start_to_seg_start = {0, 0};
  cut_start_to_seg_start[0] = segment_point1[0] - cutting_line_point1[0];
  cut_start_to_seg_start[1] = segment_point1[1] - cutting_line_point1[1];

  double cut_dir_cross_seg_dir = crossProduct2D(cut_dir, seg_dir);

  if (std::abs(cut_dir_cross_seg_dir) > Xfem::tol)
  {
    // Fraction of the distance along the cutting segment where it intersects the edge segment
    double cut_int_frac = crossProduct2D(cut_start_to_seg_start, seg_dir) / cut_dir_cross_seg_dir;

    if (cut_int_frac >= 0.0 && cut_int_frac <= 1)
    { // Cutting segment intersects the line of the edge segment, but the intersection point may
      // be
      // outside the segment
      double int_frac = crossProduct2D(cut_start_to_seg_start, cut_dir) / cut_dir_cross_seg_dir;
      if (int_frac >= 0.0 &&
          int_frac <= 1.0) // TODO: revisit end cases for intersections with corners
      {
        cut_segment = true;
      }
    }
  }

  if (cut_segment)
  {
    // line of segment_point1/segment_point2 is a1x + b1x = c1
    double a = segment_point2[1] - segment_point1[1];
    double b = segment_point1[0] - segment_point2[0];
    double c = a * (segment_point1[0]) + b * (segment_point1[1]);
    // line of cutting_line_points as a2x + b2y = c2
    double a1 = cutting_line_point2[1] - cutting_line_point1[1];
    double b1 = cutting_line_point1[0] - cutting_line_point2[0];
    double c1 = a1 * (cutting_line_point1[0]) + b1 * (cutting_line_point1[1]);
    double det = a * b1 - a1 * b;
    if (det == 0)
      cut_segment = false;
    else
    {
      intersect_Point[0] = (b1 * c - b * c1) / det;
      intersect_Point[1] = (a * c1 - a1 * c) / det;
    }
  }
  return cut_segment;
}

double
EFAFragment2D::crossProduct2D(const std::vector<double> & point_a,
                              const std::vector<double> & point_b) const
{
  return (point_a[0] * point_b[1] - point_b[0] * point_a[1]);
}

double
EFAFragment2D::distanceBetweenPoints(const std::vector<double> & point_a,
                                     const std::vector<double> & point_b) const
{
  return std::sqrt(std::pow(point_b[0] - point_a[0], 2) + std::pow(point_b[1] - point_a[1], 2));
}

std::vector<EFAFragment2D *>
EFAFragment2D::split()
{

  // This method will split one existing fragment into one or two
  // new fragments and return them. Still used for 3D
  // N.B. each boundary each can only have 1 cut at most
  std::vector<EFAFragment2D *> new_fragments;
  std::vector<std::vector<EFANode *>> fragment_nodes(
      2);                       // vectors of EFA nodes in the two fragments
  unsigned int frag_number = 0; // Index of the current fragment that we are assembling nodes into
  unsigned int edge_cut_count = 0;
  unsigned int node_cut_count = 0;
  for (unsigned int iedge = 0; iedge < _boundary_edges.size(); ++iedge)
  {
    fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getNode(0));

    if (_boundary_edges[iedge]->getNode(0)->category() ==
        EFANode::N_CATEGORY_EMBEDDED_PERMANENT) // if current node has been cut change fragment
    {
      ++node_cut_count;
      frag_number = 1 - frag_number; // Toggle between 0 and 1
      fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getNode(0));
    }

    if (_boundary_edges[iedge]->numEmbeddedNodes() > 1)
      EFAError("A fragment boundary edge can't have more than 1 cuts");
    if (_boundary_edges[iedge]->hasIntersection()) // if current edge is cut add cut intersection //
                                                   // node to both fragments and and change fragment
    {
      fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getEmbeddedNode(0));
      ++edge_cut_count;
      frag_number = 1 - frag_number; // Toggle between 0 and 1
      fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getEmbeddedNode(0));
    }
  }

  if ((edge_cut_count + node_cut_count) > 1) // any two cuts case
  {
    for (unsigned int frag_idx = 0; frag_idx < 2; ++frag_idx) // Create 2 fragments
    {
      auto & this_frag_nodes = fragment_nodes[frag_idx];
      // check to make sure an edge wasn't cut
      if (this_frag_nodes.size() >= 3)
      {
        EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
        for (unsigned int inode = 0; inode < this_frag_nodes.size() - 1; inode++)
          new_frag->addEdge(new EFAEdge(this_frag_nodes[inode], this_frag_nodes[inode + 1]));

        new_frag->addEdge(
            new EFAEdge(this_frag_nodes[this_frag_nodes.size() - 1], this_frag_nodes[0]));

        new_fragments.push_back(new_frag);
      }
    }
  }
  else if (edge_cut_count == 1) // single edge cut case
  {
    EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
    for (unsigned int inode = 0; inode < fragment_nodes[0].size() - 1;
         inode++) // assemble fragment part 1
      new_frag->addEdge(new EFAEdge(fragment_nodes[0][inode], fragment_nodes[0][inode + 1]));

    for (unsigned int inode = 0; inode < fragment_nodes[1].size() - 1;
         inode++) // assemble fragment part 2
      new_frag->addEdge(new EFAEdge(fragment_nodes[1][inode], fragment_nodes[1][inode + 1]));

    new_frag->addEdge(
        new EFAEdge(fragment_nodes[1][fragment_nodes[1].size() - 1], fragment_nodes[0][0]));

    new_fragments.push_back(new_frag);
  }
  else if (node_cut_count == 1) // single node cut case
  {
    EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
    for (unsigned int iedge = 0; iedge < _boundary_edges.size(); ++iedge)
    {
      EFANode * first_node_on_edge = _boundary_edges[iedge]->getNode(0);
      EFANode * second_node_on_edge = _boundary_edges[iedge]->getNode(1);
      new_frag->addEdge(new EFAEdge(first_node_on_edge, second_node_on_edge));
    }

    new_fragments.push_back(new_frag);
  }

  return new_fragments;
}
