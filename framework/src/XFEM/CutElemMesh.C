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

#include "EFAfuncs.h"
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
      newElem->set_node(j, currNode);
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
    newElem->set_node(j, currNode);
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
    elem->clear_neighbors();
  }

  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement* curr_elem = eit->second;
    curr_elem->setup_neighbors(InverseConnectivityMap);
  } // loop over all elements

  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->neighbor_sanity_check();
  }
}

void CutElemMesh::initCrackTipTopology()
{
  CrackTipElements.clear(); // re-build CrackTipElements!
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->init_crack_tip(CrackTipElements); // CrackTipElements changed here
  }
}

void CutElemMesh::addEdgeIntersection(unsigned int elemid, unsigned int edgeid, double position)
{
  std::map<unsigned int, EFAelement*>::iterator eit = Elements.find(elemid);
  if (eit == Elements.end())
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addEdgeIntersection")

  EFAelement *curr_elem = eit->second;
  curr_elem->add_edge_cut(edgeid, position, NULL, EmbeddedNodes);
}

void CutElemMesh::addFragEdgeIntersection(unsigned int elemid, unsigned int frag_edge_id, double position)
{
  // N.B. this method must be called after addEdgeIntersection
  std::map<unsigned int, EFAelement*>::iterator eit = Elements.find(elemid);
  if (eit == Elements.end())
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addFragEdgeIntersection")

  EFAelement *elem = eit->second;
  elem->add_frag_edge_cut(frag_edge_id, position, EmbeddedNodes);
}

void CutElemMesh::updatePhysicalLinksAndFragments() // 2D specific element
{
  //loop over the elements in the mesh
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->update_fragments(CrackTipElements, EmbeddedNodes);
  } // loop over all elements
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
    curr_elem->remove_parent_children();
    for (unsigned int j = 0; j < curr_elem->num_nodes(); j++)
    {
      EFAnode *curr_node = curr_elem->get_node(j);
      InverseConnectivityMap[curr_node].insert(curr_elem);
    }
  }

  for (unsigned int i=0; i < PermanentNodes.size(); ++i)
    PermanentNodes[i]->remove_parent();

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
  elem->restore_fragment(from_elem);
}

void CutElemMesh::restoreEdgeIntersections(EFAelement * const elem, EFAelement * const from_elem)
{
  // 2D specific method
  for (unsigned int i = 0; i < elem->num_edges(); ++i)
  {
    if (from_elem->get_edge(i)->has_intersection())
    {
      double intersection_x = from_elem->get_edge(i)->get_intersection(from_elem->get_node(i));
      EFAnode * embedded_node = from_elem->get_edge(i)->get_embedded_node();
      elem->add_edge_cut(i, intersection_x, embedded_node, EmbeddedNodes);
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
    curr_elem->create_child(CrackTipElements, Elements, newChildElements, 
                            ChildElements, ParentElements, TempNodes);
  } // loop over elements
  //Merge newChildElements back in with Elements
  Elements.insert(newChildElements.begin(),newChildElements.end());
}

void CutElemMesh::connectFragments(bool mergeUncutVirtualEdges)
{
  //now perform the comparison on the children
  for (unsigned int elem_iter = 0; elem_iter < ChildElements.size(); elem_iter++)
  {
    EFAelement *childElem = ChildElements[elem_iter];
    childElem->connect_neighbors(PermanentNodes, EmbeddedNodes, TempNodes,
                                 MergedEdgeMap, mergeUncutVirtualEdges);
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
    currElem->print_elem();
  }
}

EFAelement*
CutElemMesh::getElemByID(unsigned int id)
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
    for (unsigned int i = 0; i < curr_elem->num_nodes(); ++i)
    {
      if (curr_elem->get_node(i)->id() == node_id[i])
        counter += 1;
    }
    if (counter == curr_elem->num_nodes())
    {
      elem_id = curr_elem->id();
      break;
    }
  }
  return elem_id;
}
