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

//TODO:
//Clean up error checking in (!found_edge)
//Save fragment for uncut element ahead of crack tip to avoid renumbering if only embedded node
//Improve overlays_elem() check to not pass in edge
//Add common code to compare neighbors & fragments (replace multiple set_intersection calls)

//Handle cases other than 0 or 2 cut edges/elem (include data structure to link cut edges with cracks?)
//Allow for more than one cut on an edge
//Allow for fragments to be cut recursively
//Support other 2d linear elements (tets)
//Support 2d higher order elements
//3D
//Test more cases

#include <algorithm>
#include <iomanip>
#include <cmath>

#include "CutElemMesh.h"

CutElemMesh::CutElemMesh()
{}

CutElemMesh::~CutElemMesh()
{
  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = EmbeddedNodes.begin(); mit != EmbeddedNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    delete eit->second;
    eit->second = NULL;
  }
}

template <typename T> bool
CutElemMesh::deleteFromMap(std::map<unsigned int, T*> &theMap, T* elemToDelete)
{
  bool didIt = false;
  typename std::map<unsigned int, T*>::iterator i=theMap.find(elemToDelete->id());
  if (i != theMap.end())
  {
    delete i->second;
    theMap.erase(i);
    elemToDelete=NULL;
    didIt = true;
  }
  return didIt;
}

template <typename T> unsigned int
CutElemMesh::getNewID(std::map<unsigned int, T*> &theMap)
{
  typename std::map<unsigned int, T*>::reverse_iterator last_elem = theMap.rbegin();
  unsigned int new_elem_id = 0;
  if (last_elem != theMap.rend())
    new_elem_id=last_elem->first+1;
  return new_elem_id;
}

unsigned int CutElemMesh::addElements( std::vector< std::vector<unsigned int> > &quads )
{
  unsigned int first_id = 0;
  unsigned int num_nodes = 4;

  if (quads.size() == 0)
    CutElemMeshError("addElements called with empty vector of quads")

  for(unsigned int i = 0; i < quads.size(); ++i) {
    unsigned int new_elem_id = getNewID(Elements);
    EFAelement* newElem = new EFAelement(new_elem_id);
    Elements.insert(std::make_pair(new_elem_id,newElem));

    if (i == 0)
      first_id = new_elem_id;

    for (unsigned int j=0; j != num_nodes; j++) {
      EFAnode * currNode = NULL;
      std::map<unsigned int, EFAnode*>::iterator mit = PermanentNodes.find(quads[i][j]);
      if (mit == PermanentNodes.end()) {
        currNode = new EFAnode(quads[i][j],N_CATEGORY_PERMANENT);
        PermanentNodes.insert(std::make_pair(quads[i][j],currNode));
      } else {
        currNode = mit->second;
      }
      newElem->nodes[j] = currNode;
      InverseConnectivityMap[currNode].insert(newElem);
    }
    newElem->createEdges();
  }
  return first_id;
}

EFAelement* CutElemMesh::addElement( std::vector<unsigned int> quad, unsigned int id )
{
  unsigned int num_nodes = 4;

  std::map<unsigned int, EFAelement*>::iterator mit = Elements.find(id);
  if (mit != Elements.end())
    CutElemMeshError("In addElement element with id: "<<id<<" already exists")

  EFAelement* newElem = new EFAelement(id);
  Elements.insert(std::make_pair(id,newElem));

  for (unsigned int j=0; j != num_nodes; j++) {
    EFAnode * currNode = NULL;
    std::map<unsigned int, EFAnode*>::iterator mit = PermanentNodes.find(quad[j]);
    if (mit == PermanentNodes.end()) {
      currNode = new EFAnode(quad[j],N_CATEGORY_PERMANENT);
      PermanentNodes.insert(std::make_pair(quad[j],currNode));
    } else {
      currNode = mit->second;
    }
    newElem->nodes[j] = currNode;
    InverseConnectivityMap[currNode].insert(newElem);
  }
  newElem->createEdges();
  return newElem;
}

void CutElemMesh::updateEdgeNeighbors()
{
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement* elem = eit->second;
    for (unsigned int edge_iter = 0; edge_iter < elem->num_edges; ++edge_iter)
    {
      elem->edge_neighbors[edge_iter] = std::vector<EFAelement*>(1,NULL);
    }
  }
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement* curr_elem = eit->second;
    std::vector<EFAnode*> nodes;

    std::set<EFAelement*> neighbor_elements;
    for (unsigned int inode=0; inode<curr_elem->num_nodes; ++inode)
    {
      std::set<EFAelement*> this_node_connected_elems = InverseConnectivityMap[curr_elem->nodes[inode]];
      neighbor_elements.insert(this_node_connected_elems.begin(), this_node_connected_elems.end());
    }

    std::set<EFAelement*>::iterator eit2;
    for (eit2 = neighbor_elements.begin(); eit2 != neighbor_elements.end(); ++eit2)
    {
      if (*eit2 != curr_elem)
      {
        EFAelement *neigh_elem = *eit2;
        std::vector<EFAnode*> common_nodes;

        std::set<EFAnode*> curr_elem_nodes;
        for (unsigned int k=0; k < curr_elem->nodes.size(); k++)
          curr_elem_nodes.insert( curr_elem->nodes[k] );

        std::set<EFAnode*> neigh_elem_nodes;
        for (unsigned int k=0; k < neigh_elem->nodes.size(); k++)
          neigh_elem_nodes.insert( neigh_elem->nodes[k] );

        std::set_intersection(curr_elem_nodes.begin(), curr_elem_nodes.end(),
                              neigh_elem_nodes.begin(), neigh_elem_nodes.end(),
                              std::inserter(common_nodes,common_nodes.end()));

        if (common_nodes.size() >= 2)
        {
          bool found_edge = false;
          for (unsigned int edge_iter = 0; edge_iter < curr_elem->num_edges; ++edge_iter)
          {
            std::set<EFAnode*> edge_nodes;
            EFAnode* edge_node1 = curr_elem->edges[edge_iter]->get_node(0);
            EFAnode* edge_node2 = curr_elem->edges[edge_iter]->get_node(1);
            edge_nodes.insert(edge_node1);
            edge_nodes.insert(edge_node2);
            bool is_edge_neighbor = false;

            //Must share nodes on this edge
            if (num_common_elems(edge_nodes, common_nodes) == 2)
            {
              //Must not overlay
              if (!neigh_elem->overlays_elem(edge_node1,edge_node2))
              {
                //Fragments must match up.
                if ((curr_elem->fragments.size()>1) ||
                    (neigh_elem->fragments.size()>1))
                {
                  CutElemMeshError("in updateEdgeNeighbors: Cannot have more than 1 fragment")
                }
                else if ((curr_elem->fragments.size()==1) &&
                         (neigh_elem->fragments.size()==1))
                {
                  if (curr_elem->fragments[0]->isConnected(*neigh_elem->fragments[0]))
                    is_edge_neighbor = true;
                }
                else //If there are no fragments to match up, consider them neighbors
                     //BWS TODO: This would occur at crack tip -- maybe we want to save that fragment after all
                {
                  is_edge_neighbor = true;
                }
              }
            }

            if (is_edge_neighbor)
            {
              if (curr_elem->edge_neighbors[edge_iter][0])
              {
                if (curr_elem->edge_neighbors[edge_iter].size() > 1)
                {
                  std::cout<<"Neighbor: "<<neigh_elem->id()<<std::endl;
                  CutElemMeshError("Element "<<curr_elem->id()<<" already has 2 edge neighbors: "
                                   <<curr_elem->edge_neighbors[edge_iter][0]->id()<<" "
                                   <<curr_elem->edge_neighbors[edge_iter][1]->id())
                }
                curr_elem->edge_neighbors[edge_iter].push_back(neigh_elem);
              }
              else
              {
                curr_elem->edge_neighbors[edge_iter][0] = neigh_elem;
              }
              found_edge = true;
            }
          }
//BWS TODO: clean this error checking up
//          if (!found_edge)
//          {
//            printMesh();
//            std::cout<<"BWS curr: "<<curr_elem->id<<" neigh: "<<neigh_elem->id<<std::endl;
//            CutElemMeshError("Could not find edge for common nodes in element " << curr_elem->id);
//          }
        }
      }
    }
  }

  //Sanity check:
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    for (unsigned int edge_iter = 0; edge_iter < curr_elem->num_edges; ++edge_iter)
    {
      for (unsigned int en_iter = 0; en_iter < curr_elem->edge_neighbors[edge_iter].size(); ++en_iter)
      {
        EFAelement* neigh_elem = curr_elem->edge_neighbors[edge_iter][en_iter];
        if (neigh_elem != NULL)
        {
          bool found_neighbor = false;
          for (unsigned int edge_iter2 = 0; edge_iter2 < neigh_elem->num_edges; ++edge_iter2)
          {
            for (unsigned int en_iter2 = 0; en_iter2 < neigh_elem->edge_neighbors[edge_iter2].size(); ++en_iter2)
            {
              if (neigh_elem->edge_neighbors[edge_iter2][en_iter2] == curr_elem)
              {
                if ((en_iter2 > 1) && (en_iter > 1))
                {
                  CutElemMeshError("Element and neighbor element cannot both have >1 neighbors on a common edge")
                }
                found_neighbor = true;
                break;
              }
            }
          }
          if (!found_neighbor)
          {
            CutElemMeshError("Neighbor element doesn't recognize current element as neighbor")
          }
        }
      }
    }
  }
}

void CutElemMesh::initCrackTipTopology()
{
  CrackTipElements.clear(); // re-build CrackTipElements!
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    unsigned int num_edges = curr_elem->num_edges;
    for (unsigned int edge_iter = 0; edge_iter < num_edges; ++edge_iter)
    {
      std::vector<EFAelement*> &edge_neighbors = curr_elem->edge_neighbors[edge_iter];
      if ((edge_neighbors.size() == 2) &&
          (curr_elem->edges[edge_iter]->has_intersection()))
      {
        //Neither neighbor overlays current element.  We are on the uncut element ahead of the tip.
        //Flag neighbors as crack tip elements and add this element as their crack tip neighbor.

        EFAnode* edge_node1 = curr_elem->edges[edge_iter]->get_node(0);
        EFAnode* edge_node2 = curr_elem->edges[edge_iter]->get_node(1);

        if ((edge_neighbors[0]->overlays_elem(edge_node1,edge_node2)) ||
            (edge_neighbors[1]->overlays_elem(edge_node1,edge_node2)))
        {
          CutElemMeshError("Element has a neighbor that overlays itself")
        }

        //Make sure the current elment hasn't been flagged as a tip element
        if (curr_elem->crack_tip_split_element)
        {
          CutElemMeshError("crack_tip_split_element already flagged.  In elem: "<<curr_elem->id()
                           << " flags: "<<curr_elem->crack_tip_split_element
                           <<" "<<edge_neighbors[0]->crack_tip_split_element
                           <<" "<<edge_neighbors[1]->crack_tip_split_element)
        }

        CrackTipElements.insert(curr_elem);

        edge_neighbors[0]->crack_tip_split_element = true;
        edge_neighbors[1]->crack_tip_split_element = true;

        edge_neighbors[0]->add_crack_tip_neighbor(curr_elem);
        edge_neighbors[1]->add_crack_tip_neighbor(curr_elem);
      }
    }
  }
}

void CutElemMesh::addEdgeIntersection(unsigned int elemid, unsigned int edgeid, double position)
{
  std::map<unsigned int, EFAelement*>::iterator eit = Elements.find(elemid);
  if (eit == Elements.end())
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addEdgeIntersection")

  EFAelement *curr_elem = eit->second;
  addEdgeIntersection( curr_elem, edgeid, position );
}

void CutElemMesh::addEdgeIntersection(EFAelement * elem, unsigned int edgeid, double position, EFAnode * embedded_node)
{

  EFAnode* local_embedded_node = NULL;
  EFAnode* edge_node1 = elem->nodes[edgeid];
  unsigned int frag_id = 99999; // which fragment has the overlapping edge
  unsigned int frag_edge_id = 99999; // the id of the overlapping fragment edge
  if (embedded_node) //use the existing embedded node if it was passed in
    local_embedded_node = embedded_node;

  if (elem->edges[edgeid]->has_intersection())
  {
    if (!elem->edges[edgeid]->has_intersection_at_position(position,edge_node1) ||
        (embedded_node && elem->edges[edgeid]->get_embedded_node() != embedded_node))
    {
      CutElemMeshError("Attempting to add edge intersection when one already exists with different position or node."
                       << " elem: "<<elem->id()<<" edge: "<<edgeid<<" position: "<<position<<" old position: "
                       <<elem->edges[edgeid]->get_intersection(edge_node1))
    }
    local_embedded_node = elem->edges[edgeid]->get_embedded_node();
  }
  else // blank edge
  {
    if (!local_embedded_node)
    {
      //create the embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      local_embedded_node = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,local_embedded_node));
    }
    elem->edges[edgeid]->add_intersection(position, local_embedded_node, edge_node1);
    if (elem->getFragmentEdgeID(edgeid, frag_id, frag_edge_id)) // get frag_id, frag_edge_id
    {
      if (!elem->fragments[frag_id]->boundary_edges[frag_edge_id]->has_intersection())
        elem->fragments[frag_id]->boundary_edges[frag_edge_id]->add_intersection(position, local_embedded_node, edge_node1);
    }
  }

  for (unsigned int en_iter = 0; en_iter < elem->edge_neighbors[edgeid].size(); ++en_iter)
  {
    EFAelement *edge_neighbor = elem->edge_neighbors[edgeid][en_iter];
    if (edge_neighbor)
    {
      unsigned int num_edges_neigh = edge_neighbor->num_edges;
      bool found = false;
      for (unsigned int j=0; j<num_edges_neigh; j++)
      {
        for (unsigned int k=0; k<edge_neighbor->edge_neighbors[j].size(); k++)
        {
          if (edge_neighbor->edge_neighbors[j][k] == elem)
          {
            found = true;
            if (edge_neighbor->edges[j]->has_intersection())
            {
              if (!edge_neighbor->edges[j]->equivalent(*elem->edges[edgeid]))
              {
                CutElemMeshError("Attempting to add edge intersection when neighbor already has one that is incompatible."
                                 << " elem: "<<elem->id()<<" edge: "<<edgeid
                                 <<" neighbor: "<<edge_neighbor->id()<<" neighbor edge: "<<j)
              }
            }
            else
            {
              edge_neighbor->edges[j]->add_intersection(position, local_embedded_node, edge_node1);
              if (edge_neighbor->getFragmentEdgeID(j, frag_id, frag_edge_id)) // get frag_id, frag_edge_id
              {
                if (!edge_neighbor->fragments[frag_id]->boundary_edges[frag_edge_id]->has_intersection())
                  edge_neighbor->fragments[frag_id]->boundary_edges[frag_edge_id]->add_intersection(position,local_embedded_node,edge_node1);
              }
            }
            break;
          }
        }
      }
      if (!found)
      {
        CutElemMeshError("Neighbor Element " << edge_neighbor->id()
                         << " on edge " << edgeid << " of Element " << elem->id() << "isn't set ")
      }
    }
  }
}

void CutElemMesh::addFragEdgeIntersection(unsigned int elemid, unsigned int frag_edge_id, double position)
{
  // N.B. this method must be called after addEdgeIntersection
  std::map<unsigned int, EFAelement*>::iterator eit = Elements.find(elemid);
  if (eit == Elements.end())
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addFragEdgeIntersection")
  EFAelement *elem = eit->second;

  if (elem->fragments.size() != 1)
    CutElemMeshError("Element: "<<elemid<<" should have only 1 fragment in addFragEdgeIntersection")
  EFAnode* local_embedded_node = NULL;

  // check if this intersection coincide with any embedded node on this edge
  double tol = 1.0e-4;
  bool isValidIntersection = true;
  EFAnode* edge_node1 = elem->fragments[0]->boundary_edges[frag_edge_id]->get_node(0);
  EFAnode* edge_node2 = elem->fragments[0]->boundary_edges[frag_edge_id]->get_node(1);
  if ((std::abs(position) < tol && edge_node1->category() == N_CATEGORY_EMBEDDED) ||
      (std::abs(1.0-position) < tol && edge_node2->category() == N_CATEGORY_EMBEDDED))
    isValidIntersection = false;
 
  // add valid intersection point to an edge 
  if (isValidIntersection)
  {
    if (elem->fragments[0]->boundary_edges[frag_edge_id]->has_intersection())
    {
      if (!elem->fragments[0]->boundary_edges[frag_edge_id]->has_intersection_at_position(position,edge_node1))
        CutElemMeshError("Attempting to add fragment edge intersection when one already exists with different position or node."
                         << " elem: "<<elem->id()<<" edge: "<<frag_edge_id<<" position: "<<position<<" old position: "
                         <<elem->fragments[0]->boundary_edges[frag_edge_id]->get_intersection(edge_node1))
    }
    else // blank edge - in fact, it can only be a blank element interior edge
    {
      if (!elem->fragments[0]->boundary_edges[frag_edge_id]->is_interior_edge())
        CutElemMeshError("Attemping to add intersection to a non-interior edge. Element: "<<elemid<<" fragment_edge: " << frag_edge_id)

      //create the embedded node and add it to the fragment's boundary edge
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      local_embedded_node = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,local_embedded_node));
      elem->fragments[0]->boundary_edges[frag_edge_id]->add_intersection(position, local_embedded_node, edge_node1);
      
      //save this interior embedded node to FaceNodes
      std::vector<double> node1_para_coor(2,0.0);
      std::vector<double> node2_para_coor(2,0.0);
      if (elem->getEmbeddedNodeParaCoor(edge_node1, node1_para_coor) &&
          elem->getEmbeddedNodeParaCoor(edge_node2, node2_para_coor))
      {
        double xi  = (1.0-position)*node1_para_coor[0] + position*node2_para_coor[0];
        double eta = (1.0-position)*node1_para_coor[1] + position*node2_para_coor[1];
        elem->interior_nodes.push_back(new FaceNode(local_embedded_node, xi, eta));
      }
      else
        CutElemMeshError("elem: "<<elem->id()<<" cannot get the para coords of two end embedded nodes")
    }
    // no need to add intersection for neighbor fragment - if this fragment has a
    // neighbor fragment, the neighbor has already been treated in addEdgeIntersection;
    // for an interior edge, there is no neighbor fragment
  }
}

void CutElemMesh::updatePhysicalLinksAndFragments()
{
  //loop over the elements in the mesh
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;

    // combine the crack-tip edges in a partial fragment to a single intersected edge
    std::set<EFAelement*>::iterator sit;
    sit = CrackTipElements.find(curr_elem);
    if (sit != CrackTipElements.end()) // curr_elem is a crack tip element
    {
      if (curr_elem->fragments.size() == 1)
        curr_elem->fragments[0]->combine_tip_edges();
    }

    // if a fragment only has 1 intersection which is in an interior edge
    // remove this embedded node (MUST DO THIS AFTER combine_tip_edges())
    if (curr_elem->fragments.size() == 1 && curr_elem->fragments[0]->get_num_cuts() == 1)
    {
      for (unsigned int i = 0; i < curr_elem->fragments[0]->boundary_edges.size(); ++i)
      {
        if (curr_elem->fragments[0]->boundary_edges[i]->is_interior_edge() &&
            curr_elem->fragments[0]->boundary_edges[i]->has_intersection())
        {
          if (curr_elem->interior_nodes.size() != 1)
            CutElemMeshError("The element must have 1 interior node")
          deleteFromMap(EmbeddedNodes,curr_elem->fragments[0]->boundary_edges[i]->get_embedded_node());
          curr_elem->fragments[0]->boundary_edges[i]->remove_embedded_node(); // set pointer to NULL
          delete curr_elem->interior_nodes[0];
          curr_elem->interior_nodes.clear();
          break;
        }
      }
    }
    
    // for an element with no fragment, create one fragment identical to the element
    if (curr_elem->fragments.size() == 0)
        curr_elem->fragments.push_back(new EFAfragment(curr_elem, true, curr_elem));
    if (curr_elem->fragments.size() != 1)
      CutElemMeshError("In updatePhysicalLinksAndFragments: element "<<curr_elem->id()<<" must have 1 fragment at this point")

    // count fragment's cut edges
    unsigned int num_cut_frag_edges = curr_elem->fragments[0]->get_num_cuts();
    if (num_cut_frag_edges > 2)
      CutElemMeshError("In element "<<curr_elem->id()<<" there are more than 2 cut fragment edges")

    if (num_cut_frag_edges == 0)
    {
      if (!curr_elem->is_partial()) // delete the temp frag for an uncut elem
      {
        delete curr_elem->fragments[0];
        curr_elem->fragments.clear();
      }
      //Element has already been cut. Don't recreate fragments because we
      //would create multiple fragments to cover the entire element and
      //lose the information about what part of this element is physical.
      continue;
    }

    // split one fragment into one or two new fragments
    std::vector<EFAfragment *> new_frags = curr_elem->fragments[0]->split();
    if (new_frags.size() == 1 || new_frags.size() == 2)
    {
      delete curr_elem->fragments[0]; // delete the old fragment
      curr_elem->fragments.clear();
      for (unsigned int i = 0; i < new_frags.size(); ++i)
        curr_elem->fragments.push_back(new_frags[i]);
    }
    else
      CutElemMeshError("Number of fragments must be 1 or 2 at this point")

    physicalLinkAndFragmentSanityCheck(curr_elem);   
  } // loop over all elements
}

void CutElemMesh::physicalLinkAndFragmentSanityCheck(EFAelement *currElem)
{
  unsigned int num_edges = currElem->num_edges;
  std::vector<unsigned int> cut_edges;
  for (unsigned int iedge=0; iedge<num_edges; ++iedge)
  {
    if(currElem->edges[iedge]->has_intersection())
    {
      cut_edges.push_back(iedge);
    }
  }
  if (cut_edges.size() > 3)
    CutElemMeshError("In element "<<currElem->id()<<" more than 2 cut edges")

  std::vector<unsigned int> num_emb;
  std::vector<unsigned int> num_perm;
  for (unsigned int i=0; i<currElem->fragments.size(); ++i)
  {
    num_emb.push_back(0);
    num_perm.push_back(0);
    std::set<EFAnode*> perm_nodes;
    std::set<EFAnode*> emb_nodes;
    for (unsigned int j=0; j<currElem->fragments[i]->boundary_edges.size(); ++j)
    {
      for (unsigned int k = 0; k < 2; ++k)
      {
        EFAnode * temp_node = currElem->fragments[i]->boundary_edges[j]->get_node(k);
        if (temp_node->category() == N_CATEGORY_PERMANENT)
          perm_nodes.insert(temp_node);
        else if (temp_node->category() == N_CATEGORY_EMBEDDED)
          emb_nodes.insert(temp_node);
        else
          CutElemMeshError("Invalid node category")
      }
    }
    num_perm[i] = perm_nodes.size();
    num_emb[i] = emb_nodes.size();
  }

  unsigned int num_interior_nodes = currElem->getNumInteriorNodes();
  if (num_interior_nodes > 0 && num_interior_nodes != 1)
    CutElemMeshError("After updatePhysicalLinksAndFragments this element has "<<num_interior_nodes<<" interior nodes")

  if (cut_edges.size() == 0)
  {
    if (currElem->fragments.size() != 1 ||
        currElem->fragments[0]->boundary_edges.size() != num_edges)
    {
      CutElemMeshError("Incorrect link size for element with 0 cuts")
    }
    if (num_emb[0] != 0 || num_perm[0] != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 0 cuts")
    }
  }
  else if (cut_edges.size() == 1)
  {
    if (currElem->fragments.size() != 1 ||
        currElem->fragments[0]->boundary_edges.size() != num_edges+1)
    {
      CutElemMeshError("Incorrect link size for element with 1 cut")
    }
    if (num_emb[0] != 1 || num_perm[0] != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 1 cut")
    }
  }
  else if (cut_edges.size() == 2)
  {
    if (currElem->fragments.size() != 2 ||
        (currElem->fragments[0]->boundary_edges.size()+currElem->fragments[1]->boundary_edges.size()) != num_edges+4)
    {
      CutElemMeshError("Incorrect link size for element with 2 cuts")
    }
    if (num_emb[0] != 2 || num_emb[1] != 2 || (num_perm[0]+num_perm[1]) != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 2 cuts")
    }
  }
  else if (cut_edges.size() == 3) // TODO: not a good sanity check
  {
    if (currElem->fragments.size() == 1)
    {
      if (num_emb[0] != 3)
        CutElemMeshError("Incorrect number of embedded nodes for element with 3 cuts and 1 fragment")
    }
    else if (currElem->fragments.size() == 2)
    {
      if (num_emb[0] != 3 || num_emb[1] != 3)
        CutElemMeshError("Incorrect number of embedded nodes for element with 3 cuts and 2 fragment")
    }
  }
}

void CutElemMesh::updateTopology(bool mergeUncutVirtualEdges)
{
  // If mergeUncutVirtualEdges=true, this algorithm replicates the
  // behavior of classical XFEM.  If false, it gives the behavior of
  // the Richardson et. al. (2011) paper

  NewNodes.clear();
  ChildElements.clear();
  ParentElements.clear();
  MergedEdgeMap.clear();

  unsigned int first_new_node_id = getNewID(PermanentNodes);

  createChildElements();
  connectFragments(mergeUncutVirtualEdges);
  sanityCheck();
  findCrackTipElements();

  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
  {
    if (mit->first >= first_new_node_id)
    {
      NewNodes.push_back(mit->second);
    }
  }
}

void CutElemMesh::reset()
{
  NewNodes.clear();
  ChildElements.clear();
  ParentElements.clear();
  MergedEdgeMap.clear();
  CrackTipElements.clear();
  InverseConnectivityMap.clear();

  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  PermanentNodes.clear();
//  for (mit = EmbeddedNodes.begin(); mit != EmbeddedNodes.end(); ++mit )
//  {
//    delete mit->second;
//    mit->second = NULL;
//  }
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  TempNodes.clear();
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    delete eit->second;
    eit->second = NULL;
  }
  Elements.clear();
}

void CutElemMesh::clearAncestry()
{
  InverseConnectivityMap.clear();
  for (unsigned int i=0; i<ParentElements.size(); ++i)
  {
    if (!deleteFromMap(Elements, ParentElements[i]))
    {
      CutElemMeshError("Attempted to delete parent element: "<<ParentElements[i]->id()
                       <<" from Elements, but couldn't find it")
    }
  }
  ParentElements.clear();

  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->parent=NULL;
    curr_elem->children.clear();
    for (unsigned int j=0; j != curr_elem->num_nodes; j++)
    {
      EFAnode *curr_node = curr_elem->nodes[j];
      InverseConnectivityMap[curr_node].insert(curr_elem);
    }
  }

  for (unsigned int i=0; i<PermanentNodes.size(); ++i)
  {
    PermanentNodes[i]->remove_parent();
  }

  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  TempNodes.clear();

  NewNodes.clear();
  ChildElements.clear();

  //TODO: Sanity check to make sure that there are no nodes that are not connected
  //      to an element -- there shouldn't be any
}

void CutElemMesh::restoreFragmentInfo(EFAelement * const elem, EFAelement * const from_elem)
{
  // restore fragments
  if (elem->fragments.size() != 0)
    CutElemMeshError("in restoreFragmentInfo elements must not have any pre-existing fragments")
  for (unsigned int i = 0; i < from_elem->fragments.size(); ++i)
    elem->fragments.push_back(new EFAfragment(*from_elem->fragments[i], elem));

  // restore interior nodes
  if (elem->interior_nodes.size() != 0)
    CutElemMeshError("in restoreFragmentInfo elements must not have any pre-exsiting interior nodes")
  for (unsigned int i = 0; i < from_elem->interior_nodes.size(); ++i)
    elem->interior_nodes.push_back(new FaceNode(*from_elem->interior_nodes[i]));

  // replace all local nodes with global nodes
  for (unsigned int i = 0; i < from_elem->num_nodes; ++i)
  {
    if (from_elem->nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
      elem->switchNode(elem->nodes[i], from_elem->nodes[i], false); //EFAelement is not a child of any parent
    else
      CutElemMeshError("In restoreFragmentInfo all of from_elem's nodes must be local")
  }
}

void CutElemMesh::restoreEdgeIntersections(EFAelement * const elem, EFAelement * const from_elem)
{
  for (unsigned int i = 0; i < elem->num_edges; ++i)
  {
    if (from_elem->edges[i]->has_intersection())
    {
      double intersection_x = from_elem->edges[i]->get_intersection(from_elem->nodes[i]);
      EFAnode * embedded_node = from_elem->edges[i]->get_embedded_node();
      addEdgeIntersection(elem, i, intersection_x, embedded_node);
    }
  }
}

void CutElemMesh::createChildElements()
{
  //temporary container for new elements -- will be merged with Elements
  std::map<unsigned int, EFAelement*> newChildElements;

  //loop over the original elements in the mesh
  std::map<unsigned int, EFAelement*>::iterator eit;
  std::map<unsigned int, EFAelement*>::iterator ElementsEnd = Elements.end();
  for (eit = Elements.begin(); eit != ElementsEnd; ++eit)
  {
    EFAelement *curr_elem = eit->second;
    if (curr_elem->children.size() != 0)
      CutElemMeshError("Element cannot have existing children in createChildElements")

    unsigned int num_edges = curr_elem->num_edges;
    unsigned int num_nodes = curr_elem->num_nodes;
    unsigned int num_links = curr_elem->fragments.size();
    if (num_links > 1 || should_duplicate_for_crack_tip(curr_elem))
    {
      if (num_links > 2)
        CutElemMeshError("More than 2 fragments not yet supported")

      //set up the children
      ParentElements.push_back(curr_elem);
      for (unsigned int ichild=0; ichild<num_links; ++ichild)
      {
        unsigned int new_elem_id;
        if (newChildElements.size() == 0)
        {
          new_elem_id = getNewID(Elements);
        }
        else
        {
          new_elem_id = getNewID(newChildElements);
        }
        EFAelement* childElem = new EFAelement(new_elem_id);
        newChildElements.insert(std::make_pair(new_elem_id,childElem));

        ChildElements.push_back(childElem);
        childElem->parent = curr_elem;
        curr_elem->children.push_back(childElem);

        // get child element's nodes
        for (unsigned int j=0; j<num_nodes; j++)
        {
          if (curr_elem->fragments[ichild]->containsNode(curr_elem->nodes[j]))
            childElem->nodes[j] = curr_elem->nodes[j]; // inherit parent's node
          else // parent element's node is not in fragment
          {
            unsigned int new_node_id = getNewID(TempNodes);
            EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_TEMP,curr_elem->nodes[j]);
            TempNodes.insert(std::make_pair(new_node_id,newNode));
            childElem->nodes[j] = newNode; // be a temp node
          }
        }

        // get child element's fragments
        EFAfragment * new_frag = new EFAfragment(childElem, true, curr_elem, ichild);
        childElem->fragments.push_back(new_frag);

        // get child element's edges
        for (unsigned int j=0; j<num_edges; j++)
        {
          unsigned int jplus1(j < (num_edges-1) ? j+1 : 0);
          EFAedge * new_edge = new EFAedge(childElem->nodes[j], childElem->nodes[jplus1]);
          if (curr_elem->edges[j]->has_intersection())
          {
            double child_position = curr_elem->edges[j]->get_intersection(curr_elem->nodes[j]);
            EFAnode * child_embedded_node = curr_elem->edges[j]->get_embedded_node();
            new_edge->add_intersection(child_position, child_embedded_node, childElem->nodes[j]);
          }
          childElem->edges[j] = new_edge;
        }
        for (unsigned int j = 0; j < num_edges; ++j) // remove embedded node on phantom edge
          if (childElem->is_edge_phantom(j) && childElem->edges[j]->has_intersection())
            childElem->edges[j]->remove_embedded_node();

        // inherit old interior nodes
        for (unsigned int j = 0; j < curr_elem->interior_nodes.size(); ++j)
          childElem->interior_nodes.push_back(new FaceNode(*curr_elem->interior_nodes[j]));
      }
    }
    else //num_links == 1 || num_links == 0
    {
      //child is itself - but don't insert into the list of ChildElements!!!
      curr_elem->children.push_back(curr_elem);
    }
  }
  //Merge newChildElements back in with Elements
  Elements.insert(newChildElements.begin(),newChildElements.end());
}

void CutElemMesh::connectFragments(bool mergeUncutVirtualEdges)
{
  //now perform the comparison on the children
  for (unsigned int elem_iter = 0; elem_iter < ChildElements.size(); elem_iter++)
  {
    EFAelement *childElem = ChildElements[elem_iter];
    EFAelement *parentElem = childElem->parent;
    unsigned int num_edges = childElem->num_edges;

    //Gather the neighbor elements and edge ids of the common edges for those neighbors
    std::vector<std::vector<EFAelement *> > NeighborElem;
    std::vector<std::vector<unsigned int> > neighbor_common_edge;
    for (unsigned int j=0; j<num_edges; ++j)
    {
      //loop over the children for the neighbor on this edge
      unsigned int num_edge_neighbors=parentElem->edge_neighbors[j].size();
      NeighborElem.push_back(std::vector<EFAelement*> (num_edge_neighbors,NULL));
      neighbor_common_edge.push_back(std::vector<unsigned int> (num_edge_neighbors,99999)); //big number to indicate edge not found

      for (unsigned int k=0; k<num_edge_neighbors; ++k)
      {
        if (parentElem->edge_neighbors[j][k])
        {
          NeighborElem[j][k] = parentElem->edge_neighbors[j][k];
          //identify the common cut edge
          bool found(false);
          unsigned int neighbor_num_edges = NeighborElem[j][k]->num_edges;
          for (unsigned int m=0; m<neighbor_num_edges; ++m)
          {
            for (unsigned int n=0; n<NeighborElem[j][k]->edge_neighbors[m].size(); ++n)
            {
              if (NeighborElem[j][k]->edge_neighbors[m][n] == parentElem)
              {
                neighbor_common_edge[j][k] = m;
                found = true;
                break;
              }
            }
            if (found) break;
          }
          if (!found)
          {
            CutElemMeshError("Parent element: "<<parentElem->id()
                             <<" is not a neighbor of its neighbor element: " << NeighborElem[j][k]->id())
          }
        }
      }
    }

    //First loop through edges and merge nodes with neighbors as appropriate
    for (unsigned int j=0; j<num_edges; ++j)
    {
      for (unsigned int k=0; k<NeighborElem[j].size(); ++k)
      {
        if (NeighborElem[j][k])
        {
          //get nodes on this edge of childElem
          std::vector<EFAnode*> childEdgeNodes;
          childEdgeNodes.push_back(childElem->edges[j]->get_node(0));
          childEdgeNodes.push_back(childElem->edges[j]->get_node(1));

          if (childElem->edges[j]->has_intersection())
          {
            for (unsigned int l=0; l < NeighborElem[j][k]->children.size(); l++)
            {
              EFAelement *childOfNeighborElem = NeighborElem[j][k]->children[l];

              //get nodes on this edge of childOfNeighborElem
              std::vector<EFAnode*> childOfNeighborEdgeNodes;
              EFAedge *neighborChildEdge = childOfNeighborElem->edges[neighbor_common_edge[j][k]];
              childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(0));
              childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(1));

              //Check to see if the nodes are already merged.  There's nothing else to do in that case.
              if (childEdgeNodes[0] == childOfNeighborEdgeNodes[1] &&
                  childEdgeNodes[1] == childOfNeighborEdgeNodes[0])
              {
                addToMergedEdgeMap(childEdgeNodes[0], childEdgeNodes[1], childElem, childOfNeighborElem);
                continue;
              }

              if (childElem->fragments[0]->isConnected(*childOfNeighborElem->fragments[0]))
              {
                std::vector<EFAnode*> merged_nodes;
                unsigned int num_edge_nodes = 2;
                for (unsigned int i=0; i<num_edge_nodes; ++i)
                {
                  unsigned int childNodeIndex = i;
                  unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

                  EFAnode* childNode = childEdgeNodes[childNodeIndex];
                  EFAnode* childOfNeighborNode = childOfNeighborEdgeNodes[neighborChildNodeIndex];

                  mergeNodes(childNode,childOfNeighborNode,childElem,childOfNeighborElem);
                  merged_nodes.push_back(childNode);
                }
                addToMergedEdgeMap(merged_nodes[0], merged_nodes[1], childElem, childOfNeighborElem);

                duplicateEmbeddedNode(childElem,childOfNeighborElem,j,neighbor_common_edge[j][k]);
              }
            } // loop over NeighborElem[j][k]'s children
          }
          else //No edge intersection -- optionally merge non-material nodes if they share a common parent
          {
            if (mergeUncutVirtualEdges && NeighborElem[j][k])
            {
              for (unsigned int l=0; l < NeighborElem[j][k]->children.size(); l++)
              {
                EFAelement *childOfNeighborElem = NeighborElem[j][k]->children[l];
                EFAedge *neighborChildEdge = childOfNeighborElem->edges[neighbor_common_edge[j][k]];

                if (!neighborChildEdge->has_intersection()) //neighbor edge must NOT have intersection either
                {
                  //get nodes on this edge of childOfNeighborElem
                  std::vector<EFAnode*> childOfNeighborEdgeNodes;
                  childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(0));
                  childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(1));

                  //Check to see if the nodes are already merged.  There's nothing else to do in that case.
                  if (childEdgeNodes[0] == childOfNeighborEdgeNodes[1] &&
                      childEdgeNodes[1] == childOfNeighborEdgeNodes[0])
                    continue;

                  unsigned int num_edge_nodes = 2;
                  for (unsigned int i=0; i<num_edge_nodes; ++i)
                  {
                    unsigned int childNodeIndex = i;
                    unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

                    EFAnode* childNode = childEdgeNodes[childNodeIndex];
                    EFAnode* childOfNeighborNode = childOfNeighborEdgeNodes[neighborChildNodeIndex];

                    if (childNode->parent() != NULL &&
                        childNode->parent() == childOfNeighborNode->parent()) //non-material node and both come from same parent
                    {
                      mergeNodes(childNode,childOfNeighborNode,childElem,childOfNeighborElem);
                    }
                  }
                }
              } // loop over NeighborElem[j][k]'s children
            }
          }
        } // if NeighborElem[j][k]
      } // loop over neighbors in edge j
    } // loop over all edges

    //Now do a second loop through edges and convert remaining nodes to permanent nodes.
    //If there is no neighbor on that edge, also duplicate the embedded node if it exists
    for (unsigned int j=0; j<num_edges; j++)
    {
      EFAnode* childNode = childElem->nodes[j];

      if(childNode->category() == N_CATEGORY_TEMP)
      {
        unsigned int new_node_id = getNewID(PermanentNodes);
        EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_PERMANENT,childNode->parent());
        PermanentNodes.insert(std::make_pair(new_node_id,newNode));

        childElem->switchNode(newNode, childNode);
        if (!deleteFromMap(TempNodes, childNode))
        {
          CutElemMeshError("Attempted to delete node: "<<childNode->id()
                           <<" from TempNodes, but couldn't find it")
        }
      }
      if ((NeighborElem[j].size() == 1) && (!NeighborElem[j][0]) &&
          (childElem->edges[j]->has_intersection()))
            //No neighbor of parent on this edge -- free edge -- need to convert to permanent nodes
      {
        duplicateEmbeddedNode(childElem,j);
      }
    }
    
    //duplicate the interior embedded node
    if (childElem->getNumInteriorNodes() > 0)
      duplicateInteriorEmbeddedNode(childElem);
  } // loop over child elements

  std::vector<EFAelement*>::iterator vit;
  for (vit=ChildElements.begin(); vit!=ChildElements.end(); )
  {
    if (*vit==NULL)
    {
      vit=ChildElements.erase(vit);
    }
    else
    {
      ++vit;
    }
  }
}

void CutElemMesh::mergeNodes(EFAnode*  &childNode,
                             EFAnode* &childOfNeighborNode,
                             EFAelement* childElem,
                             EFAelement* childOfNeighborElem)
{
  if (childNode != childOfNeighborNode)
  {
    if(childNode->category() == N_CATEGORY_PERMANENT)
    {
      if(childOfNeighborNode->category() == N_CATEGORY_PERMANENT)
      {
        if (childOfNeighborNode->parent() == childNode) // merge into childNode
        {
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
          if (!deleteFromMap(PermanentNodes, childOfNeighborNode))
          {
            CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id()
                             <<" from PermanentNodes, but couldn't find it")
          }
          childOfNeighborNode = childNode;
        }
        else if (childNode->parent() == childOfNeighborNode) // merge into childOfNeighborNode
        {
          childElem->switchNode(childOfNeighborNode, childNode);
          if (!deleteFromMap(PermanentNodes, childNode))
          {
            CutElemMeshError("Attempted to delete node: "<<childNode->id()
                             <<" from PermanentNodes, but couldn't find it")
          }
          childNode = childOfNeighborNode;
        }
        else if (childNode->parent() != NULL && childNode->parent() == childOfNeighborNode->parent())
        {
          // merge into childNode if both nodes are child permanent
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
          if (!deleteFromMap(PermanentNodes, childOfNeighborNode)) // delete childOfNeighborNode
          {
            CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id()
                             <<" from PermanentNodes, but couldn't find it")
          }
          childOfNeighborNode = childNode;
        }
        else
        {
          CutElemMeshError("Attempting to merge nodes: "<<childNode->id()<<" and "
                           <<childOfNeighborNode->id()<<" but both are permanent themselves")
        }
      }
      else
      {
        if (childOfNeighborNode->parent() != childNode &&
            childOfNeighborNode->parent() != childNode->parent())
        {
          CutElemMeshError("Attempting to merge nodes "<<childOfNeighborNode->id_cat_str()<<" and "
                           <<childNode->id_cat_str()<<" but neither the 2nd node nor its parent is parent of the 1st")
        }
        childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
        if (!deleteFromMap(TempNodes, childOfNeighborNode))
        {
          CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id()
                           <<" from TempNodes, but couldn't find it")
        }
        childOfNeighborNode = childNode;
      }
    }
    else if(childOfNeighborNode->category() == N_CATEGORY_PERMANENT)
    {
      if (childNode->parent() != childOfNeighborNode &&
          childNode->parent() != childOfNeighborNode->parent())
      {
        CutElemMeshError("Attempting to merge nodes "<<childNode->id()<<" and "
                         <<childOfNeighborNode->id()<<" but neither the 2nd node nor its parent is parent of the 1st")
      }
      childElem->switchNode(childOfNeighborNode, childNode);
      if (!deleteFromMap(TempNodes, childNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childNode->id()<<" from TempNodes, but couldn't find it")
      }
      childNode = childOfNeighborNode;
    }
    else //both nodes are temporary -- create new permanent node and delete temporary nodes
    {
      unsigned int new_node_id = getNewID(PermanentNodes);
      EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_PERMANENT,childNode->parent());
      PermanentNodes.insert(std::make_pair(new_node_id,newNode));

      childOfNeighborElem->switchNode(newNode, childOfNeighborNode);
      childElem->switchNode(newNode, childNode);

      if (childNode->parent() != childOfNeighborNode->parent())
      {
        CutElemMeshError("Attempting to merge nodes "<<childNode->id()<<" and "
                         <<childOfNeighborNode->id()<<" but they don't share a common parent")
      }

      if (!deleteFromMap(TempNodes, childOfNeighborNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id()
                         <<" from TempNodes, but couldn't find it")
      }
      if (!deleteFromMap(TempNodes, childNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childNode->id()
                         <<" from TempNodes, but couldn't find it")
      }
      childOfNeighborNode = newNode;
      childNode = newNode;
    }
  }
}

void CutElemMesh::addToMergedEdgeMap(EFAnode* node1,
                                     EFAnode* node2,
                                     EFAelement* elem1,
                                     EFAelement* elem2)
{
  std::set<EFAnode*> edge_nodes;
  edge_nodes.insert(node1);
  edge_nodes.insert(node2);
  MergedEdgeMap[edge_nodes].insert(elem1);
  MergedEdgeMap[edge_nodes].insert(elem2);
}

//This version is for cases where there is a neighbor element
void CutElemMesh::duplicateEmbeddedNode(EFAelement* currElem,
                                        EFAelement* neighborElem,
                                        unsigned int edgeID,
                                        unsigned int neighborEdgeID)
{
  EFAnode* embeddedNode = currElem->edges[edgeID]->get_embedded_node();
  EFAnode* neighEmbeddedNode = neighborElem->edges[neighborEdgeID]->get_embedded_node();

  if (embeddedNode != neighEmbeddedNode)
    CutElemMeshError("Embedded nodes on merged edge must match")

  //Check to see whether the embedded node can be duplicated for both the current element
  //and the neighbor element.

  bool current_split = false;   //Is the current element being split on this edge?
  bool current_tip = false;     //Is the current element a crack tip element with a split on this edge?
  bool neighbor_split = false;  //Is the neighbor element being split on this edge?
  bool neighbor_tip = false;    //Is the neighbor element a crack tip element with a split on this edge?

  if (currElem->parent &&
      currElem->parent->children.size() >= 1)
  {
    if (currElem->parent->children.size() > 1)
    {
      bool hasSiblingWithSameEmbeddedNode = false;
      for (unsigned int i=0; i<currElem->parent->children.size(); ++i)
      {
        if (currElem->parent->children[i] != currElem &&
            currElem->parent->children[i]->edges[edgeID]->get_embedded_node() == embeddedNode)
        {
          hasSiblingWithSameEmbeddedNode = true;
          break;
        }
      }
      if (hasSiblingWithSameEmbeddedNode)
        current_split = true;
    }
    else if (currElem->parent->crack_tip_split_element)
    {
      for (unsigned int i=0; i<currElem->parent->crack_tip_neighbors.size(); ++i)
      {
        if (currElem->parent->crack_tip_neighbors[i] == edgeID)
        {
          current_tip = true;
          break;
        }
      }
    }
  }

  if (neighborElem->parent &&
      neighborElem->parent->children.size() >= 1)
  {
    if (neighborElem->parent->children.size() > 1)
    {
      bool hasSiblingWithSameEmbeddedNode = false;
      for (unsigned int i=0; i<neighborElem->parent->children.size(); ++i)
      {
        if (neighborElem->parent->children[i] != neighborElem &&
            neighborElem->parent->children[i]->edges[neighborEdgeID]->get_embedded_node() == embeddedNode)
        {
          hasSiblingWithSameEmbeddedNode = true;
          break;
        }
      }
      if (hasSiblingWithSameEmbeddedNode)
        neighbor_split = true;
    }
    else if (neighborElem->parent->crack_tip_split_element)
    {
      for (unsigned int i=0; i<neighborElem->parent->crack_tip_neighbors.size(); ++i)
      {
        if (neighborElem->parent->crack_tip_neighbors[i] == neighborEdgeID)
        {
          neighbor_tip = true;
          break;
        }
      }
    }
  }

  bool can_dup = false;
  // Don't duplicate if current and neighbor are both crack tip elems
  // because that embedded node has already been duplicated
  if ((current_split && neighbor_split) ||
      (current_split && neighbor_tip) ||
      (neighbor_split && current_tip))
  {
    can_dup = true;
  }

  if (can_dup)
  {
    // Determine whether the split occurs on the current embedded node.
    // This would happen if the link set contains the embedded node
    // and only one of the nodes on the edge.
    std::vector<EFAnode*> currCommonNodes = currElem->fragments[0]->commonNodesWithEdge(*currElem->edges[edgeID]);
    std::vector<EFAnode*> neighCommonNodes = neighborElem->fragments[0]->commonNodesWithEdge(*currElem->edges[edgeID]);   

    if (currElem->fragments[0]->containsNode(embeddedNode) &&
        neighborElem->fragments[0]->containsNode(embeddedNode) &&
        currCommonNodes.size() == 1 && neighCommonNodes.size() == 1)
    {
      // Duplicate this embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));

      currElem->switchEmbeddedNode(newNode, embeddedNode);
      neighborElem->switchEmbeddedNode(newNode, embeddedNode);
    }
  }
}

//This version is for cases when there is no neighbor
void CutElemMesh::duplicateEmbeddedNode(EFAelement* currElem,
                                        unsigned int edgeID)
{
  EFAnode* embeddedNode = currElem->edges[edgeID]->get_embedded_node();

  // Do elements on both side of the common edge have siblings?
  if (currElem->parent &&
      currElem->parent->children.size() > 1)
  {
    // Determine whether any of the sibling child elements have the same
    // embedded node.  Only duplicate if that is the case.

    bool currElemHasSiblingWithSameEmbeddedNode = false;
    for (unsigned int i=0; i<currElem->parent->children.size(); ++i)
    {
      if (currElem->parent->children[i] != currElem &&
          currElem->parent->children[i]->edges[edgeID]->get_embedded_node() == embeddedNode)
      {
        currElemHasSiblingWithSameEmbeddedNode = true;
        break;
      }
    }

    if (currElemHasSiblingWithSameEmbeddedNode)
    {
      // Determine whether the split occurs on the current embedded node.
      // This would happen if the link set contains the embedded node
      // and only one of the nodes on the edge.

      std::vector<EFAnode*> currCommonNodes = currElem->fragments[0]->commonNodesWithEdge(*currElem->edges[edgeID]);
      
      if (currCommonNodes.size() == 1 &&
          currElem->fragments[0]->containsNode(embeddedNode))
      {
        // Duplicate this embedded node
        unsigned int new_node_id = getNewID(EmbeddedNodes);
        EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
        EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));

        currElem->switchEmbeddedNode(newNode, embeddedNode);
      }
    }
  }
}

void CutElemMesh::duplicateInteriorEmbeddedNode(EFAelement* currElem)
{
  if (currElem->getNumInteriorNodes() != 1)
    CutElemMeshError("current elem must have 1 interior node")
  EFAnode* embeddedNode = currElem->interior_nodes[0]->get_node();

  if (currElem->parent &&
      currElem->parent->children.size() > 1)
  {
    // Determine whether any of the sibling child elements have the same
    // embedded node.  Only duplicate if that is the case.

    bool currElemHasSiblingWithSameEmbeddedNode = false;
    for (unsigned int i=0; i<currElem->parent->children.size(); ++i)
    {
      if (currElem->parent->children[i] != currElem)
      {
        if (currElem->parent->children[i]->getNumInteriorNodes() != 1)
          CutElemMeshError("sibling elem must have 1 interior node")
        if(currElem->parent->children[i]->interior_nodes[0]->get_node() == embeddedNode)
        {
          currElemHasSiblingWithSameEmbeddedNode = true;
          break;
        }
      }
    }

    if (currElemHasSiblingWithSameEmbeddedNode)
    {
      // Duplicate this embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));

      currElem->switchEmbeddedNode(newNode, embeddedNode);
    }
  }
}

bool CutElemMesh::should_duplicate_for_crack_tip(EFAelement* currElem)
{
  // This method is called in createChildElements()
  // Only duplicate when 
  // 1) currElem will be a NEW crack tip element with partial fragment
  // 2) currElem is a crack tip split element at last time step and the tip will extend
  // 3) currElem is the neighbor of a to-be-second-split element which has another neighbor
  //    sharing a phantom node with currElem
  bool should_duplicate = false;
  std::set<EFAelement*>::iterator sit;
  sit = CrackTipElements.find(currElem);
  if (sit == CrackTipElements.end() && currElem->frag_has_tip_edges() && currElem->is_partial())
    should_duplicate = true;
  else if (currElem->shouldDuplicateCrackTipSplitElem())
    should_duplicate = true;
  else if (currElem->shouldDuplicateForPhantomCorner())
    should_duplicate = true;
  else {}

  return should_duplicate;
}

void CutElemMesh::sanityCheck()
{
  //Make sure there are no remaining TempNodes
  if (TempNodes.size()>0)
  {
    std::cout<<"TempNodes size > 0.  size="<<TempNodes.size()<<std::endl;
    printMesh();
    exit(1);
  }
}

void CutElemMesh::findCrackTipElements()
{
  std::set<EFAelement*>::iterator sit;
  //Delete all elements that were previously flagged as crack tip elements if they have
  //been split (and hence appear in ParentElements).
  for (unsigned int i=0; i<ParentElements.size(); ++i)
  {
    sit = CrackTipElements.find(ParentElements[i]);
    if (sit != CrackTipElements.end())
    {
      CrackTipElements.erase(sit);
    }
  }

  //Debug: print MergedEdgeMap
  std::map<std::set<EFAnode*>, std::set<EFAelement*> >::iterator memit;
  //std::cout<<"MergedEdgeMap:"<<std::endl;
  //for (memit = MergedEdgeMap.begin(); memit != MergedEdgeMap.end(); ++memit)
  //{
  //  std::cout<<"Elems: ";
  //  std::set<element_t*> conn_elems = memit->second;
  //  std::set<element_t*>::iterator setit;
  //  for (setit = conn_elems.begin(); setit != conn_elems.end(); ++setit)
  //  {
  //    std::cout<<(*setit)->id<<" ";
  //  }
  //  std::cout<<std::endl;
  //}

  //Go through MergedEdgeMap to find elements that are newly at the crack tip due to
  //crack growth.
  for (memit = MergedEdgeMap.begin(); memit != MergedEdgeMap.end(); ++memit)
  {
    if (memit->second.size() < 2)
    {
      CutElemMeshError("in findCrackTipElements() cannot have <2 elements on common edge")
    }
    else if (memit->second.size() == 2)
    {
    }
    else if (memit->second.size() == 3)
    {
      std::vector< EFAelement* > this_tip_elems(memit->second.begin(),memit->second.end());
      bool olay01 = this_tip_elems[0]->overlays_elem(this_tip_elems[1]);
      bool olay12 = this_tip_elems[1]->overlays_elem(this_tip_elems[2]);
      bool olay20 = this_tip_elems[2]->overlays_elem(this_tip_elems[0]);

      if (olay01)
      {
        if (olay12 || olay20)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay")
        }
        CrackTipElements.insert(this_tip_elems[2]);
      }
      else if (olay12)
      {
        if (olay01 || olay20)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay")
        }
        CrackTipElements.insert(this_tip_elems[0]);
      }
      else if (olay20)
      {
        if (olay01 || olay12)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay")
        }
        CrackTipElements.insert(this_tip_elems[1]);
      }
    }
  }
  //std::cout<<"Crack tip elements: ";
  //for (sit=CrackTipElements.begin(); sit!=CrackTipElements.end(); ++sit)
  //{
  //  std::cout<<(*sit)->id<<" ";
  //}
  //std::cout<<std::endl;
}

void CutElemMesh::printMesh()
{
  std::cout<<"============================================================"
           <<"=================================================="<<std::endl;
  std::cout<<"                                            CutElemMesh Data"<<std::endl;
  std::cout<<"============================================================"
           <<"=================================================="<<std::endl;
  std::cout << "Permanent Nodes:" << std::endl;
  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
    std::cout << "  " << mit->second->id() << std::endl;
  std::cout << "Temp Nodes:" << std::endl;
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
    std::cout << "  " << mit->second->id() << std::endl;
  std::cout << "Embedded Nodes:" << std::endl;
  for (mit = EmbeddedNodes.begin(); mit != EmbeddedNodes.end(); ++mit )
    std::cout << "  " << mit->second->id() << std::endl;
  std::cout << "Parent Elements:" << std::endl;
  for (unsigned int i=0; i<ParentElements.size(); ++i)
    std::cout << " " << ParentElements[i]->id() << std::endl;
  std::cout << "Child Elements:" << std::endl;
  for (unsigned int i=0; i<ChildElements.size(); ++i)
    std::cout << " " << ChildElements[i]->id() << std::endl;
  std::cout << "Elements:" << std::endl;
  std::cout << "  id "
            << "|  nodes                "
            << "|  embedded nodes       "
            << "|  edge neighbors       "
            << "|  frag "
            << "|  frag link      ...   "
            << std::endl;
  std::cout<<"------------------------------------------------------------"
           <<"--------------------------------------------------"<<std::endl;
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    EFAelement* currElem = eit->second;
    std::cout << std::setw(4);
    std::cout << currElem->id() << " | ";
    for (unsigned int j=0; j<currElem->num_nodes; j++)
    {
      std::cout << std::setw(5) << currElem->nodes[j]->id_cat_str();
    }

    std::cout << "  | ";
    for (unsigned int j=0; j<currElem->num_edges; j++)
    {
      std::cout<<std::setw(4);
      if (currElem->edges[j]->has_intersection())
        std::cout << currElem->edges[j]->get_embedded_node()->id() << " ";
      else
        std::cout << "  -- ";
    }
    std::cout << "  | ";
    for (unsigned int j=0; j<currElem->num_edges; j++)
    {
      if (currElem->edge_neighbors[j].size() > 1)
      {
        std::cout << "[";
        for (unsigned int k=0; k<currElem->edge_neighbors[j].size(); ++k)
        {
          std::cout << currElem->edge_neighbors[j][k]->id();
          if (k == currElem->edge_neighbors[j].size()-1)
            std::cout<<"]";
          else
            std::cout<<" ";
        }
        std::cout<< " ";
      }
      else
      {
        std::cout<<std::setw(4);
        if (currElem->edge_neighbors[j][0])
          std::cout << currElem->edge_neighbors[j][0]->id() << " ";
        else
          std::cout << "  -- ";
      }
    }
    std::cout << "  | ";
    for (unsigned int j=0; j!= currElem->fragments.size(); j++)
    {
      std::cout<<std::setw(4);
      std::cout << " " << j << " | ";
      for (unsigned int k=0; k < currElem->fragments[j]->boundary_edges.size(); k++)
      {
        EFAnode* prt_node = currElem->fragments[j]->boundary_edges[k]->get_node(0);
        unsigned int kprev(k>0 ? k-1 : currElem->fragments[j]->boundary_edges.size()-1);
        if (!currElem->fragments[j]->boundary_edges[kprev]->containsNode(prt_node))
          prt_node = currElem->fragments[j]->boundary_edges[k]->get_node(1);
        std::cout << std::setw(5) << prt_node->id_cat_str();
      }
    }
    std::cout << std::endl;
  }
}

EFAelement* CutElemMesh::getElemByID(unsigned int id)
{
  std::map<unsigned int, EFAelement*>::iterator mit = Elements.find(id);
  if (mit == Elements.end())
    CutElemMeshError("in getElemByID() could not find element: "<<id)
  return mit->second;
}

unsigned int
CutElemMesh::getElemIdByNodes(unsigned int * node_id)
{
  unsigned int elem_id = 99999;
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    unsigned int counter = 0;
    for (unsigned int i = 0; i < curr_elem->num_nodes; ++i)
    {
      if (curr_elem->nodes[i]->id() == node_id[i])
        counter += 1;
    }
    if (counter == curr_elem->num_nodes)
    {
      elem_id = curr_elem->id();
      break;
    }
  }
  return elem_id;
}

template <class T>
unsigned int 
CutElemMesh::num_common_elems(std::set<T> &v1, std::vector<T> &v2)
{
  std::vector<T> common_elems;
  std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                        std::inserter(common_elems, common_elems.end()));
  return common_elems.size();
}
