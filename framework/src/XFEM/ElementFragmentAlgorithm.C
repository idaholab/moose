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
#include "ElementFragmentAlgorithm.h"

ElementFragmentAlgorithm::ElementFragmentAlgorithm()
{}

ElementFragmentAlgorithm::~ElementFragmentAlgorithm()
{
  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = _embedded_nodes.begin(); mit != _embedded_nodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit )
  {
    delete eit->second;
    eit->second = NULL;
  }
}

unsigned int
ElementFragmentAlgorithm::addElements( std::vector< std::vector<unsigned int> > &quads )
{
  unsigned int first_id = 0;
  unsigned int num_nodes = quads[0].size();

  if (quads.size() == 0)
    CutElemMeshError("addElements called with empty vector of quads")

  for(unsigned int i = 0; i < quads.size(); ++i)
  {
    unsigned int new_elem_id = getNewID(_elements);
    EFAelement* newElem = new EFAelement(new_elem_id, num_nodes);
    _elements.insert(std::make_pair(new_elem_id,newElem));

    if (i == 0)
      first_id = new_elem_id;

    for (unsigned int j = 0; j < num_nodes; ++j)
    {
      EFAnode * currNode = NULL;
      std::map<unsigned int, EFAnode*>::iterator mit = _permanent_nodes.find(quads[i][j]);
      if (mit == _permanent_nodes.end())
      {
        currNode = new EFAnode(quads[i][j],N_CATEGORY_PERMANENT);
        _permanent_nodes.insert(std::make_pair(quads[i][j],currNode));
      }
      else
      {
        currNode = mit->second;
      }
      newElem->set_node(j, currNode);
      _inverse_connectivity[currNode].insert(newElem);
    }
    newElem->createEdges();
  }
  return first_id;
}

EFAelement*
ElementFragmentAlgorithm::addElement( std::vector<unsigned int> quad, unsigned int id )
{
  unsigned int num_nodes = quad.size();

  std::map<unsigned int, EFAelement*>::iterator mit = _elements.find(id);
  if (mit != _elements.end())
    CutElemMeshError("In addElement element with id: "<<id<<" already exists")

  EFAelement* newElem = new EFAelement(id, num_nodes);
  _elements.insert(std::make_pair(id,newElem));

  for (unsigned int j = 0; j < num_nodes; ++j)
  {
    EFAnode * currNode = NULL;
    std::map<unsigned int, EFAnode*>::iterator mit = _permanent_nodes.find(quad[j]);
    if (mit == _permanent_nodes.end())
    {
      currNode = new EFAnode(quad[j],N_CATEGORY_PERMANENT);
      _permanent_nodes.insert(std::make_pair(quad[j],currNode));
    }
    else
    {
      currNode = mit->second;
    }
    newElem->set_node(j, currNode);
    _inverse_connectivity[currNode].insert(newElem);
  }
  newElem->createEdges();
  return newElem;
}

void
ElementFragmentAlgorithm::set_dimension(unsigned int ndm)
{
  _mesh_dim = ndm;
}

void
ElementFragmentAlgorithm::updateEdgeNeighbors()
{
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAelement* elem = eit->second;
    elem->clear_neighbors();
  }

  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAelement* curr_elem = eit->second;
    curr_elem->setup_neighbors(_inverse_connectivity);
  } // loop over all elements

  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->neighbor_sanity_check();
  }
}

void
ElementFragmentAlgorithm::initCrackTipTopology()
{
  _crack_tip_elements.clear(); // re-build CrackTipElements!
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->init_crack_tip(_crack_tip_elements); // CrackTipElements changed here
  }
}

void
ElementFragmentAlgorithm::addElemEdgeIntersection(unsigned int elemid, unsigned int edgeid, double position)
{
  // this method is called when we are marking cut edges
  std::map<unsigned int, EFAelement*>::iterator eit = _elements.find(elemid);
  if (eit == _elements.end())
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addEdgeIntersection")

  EFAelement *curr_elem = eit->second;
  curr_elem->add_edge_cut(edgeid, position, NULL, _embedded_nodes, true);
}

void
ElementFragmentAlgorithm::addFragEdgeIntersection(unsigned int elemid, unsigned int frag_edge_id, double position)
{
  // N.B. this method must be called after addEdgeIntersection
  std::map<unsigned int, EFAelement*>::iterator eit = _elements.find(elemid);
  if (eit == _elements.end())
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addFragEdgeIntersection")

  EFAelement *elem = eit->second;
  elem->add_frag_edge_cut(frag_edge_id, position, _embedded_nodes);
}

void
ElementFragmentAlgorithm::updatePhysicalLinksAndFragments()
{
  //loop over the elements in the mesh
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->update_fragments(_crack_tip_elements, _embedded_nodes);
  } // loop over all elements
}

void
ElementFragmentAlgorithm::updateTopology(bool mergeUncutVirtualEdges)
{
  // If mergeUncutVirtualEdges=true, this algorithm replicates the
  // behavior of classical XFEM.  If false, it gives the behavior of
  // the Richardson et. al. (2011) paper

  _new_nodes.clear();
  _child_elements.clear();
  _parent_elements.clear();
//  _merged_edge_map.clear();

  unsigned int first_new_node_id = getNewID(_permanent_nodes);

  createChildElements();
  connectFragments(mergeUncutVirtualEdges);
  sanityCheck();
  findCrackTipElements();

  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit )
  {
    if (mit->first >= first_new_node_id)
    {
      _new_nodes.push_back(mit->second);
    }
  }
}

void
ElementFragmentAlgorithm::reset()
{
  _mesh_dim = 0;
  _new_nodes.clear();
  _child_elements.clear();
  _parent_elements.clear();
//  _merged_edge_map.clear();
  _crack_tip_elements.clear();
  _inverse_connectivity.clear();

  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  _permanent_nodes.clear();
//  for (mit = EmbeddedNodes.begin(); mit != EmbeddedNodes.end(); ++mit )
//  {
//    delete mit->second;
//    mit->second = NULL;
//  }
  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  _temp_nodes.clear();
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit )
  {
    delete eit->second;
    eit->second = NULL;
  }
  _elements.clear();
}

void
ElementFragmentAlgorithm::clearAncestry()
{
  _inverse_connectivity.clear();
  for (unsigned int i = 0; i < _parent_elements.size(); ++i)
  {
    if (!deleteFromMap(_elements, _parent_elements[i]))
      CutElemMeshError("Attempted to delete parent element: "<<_parent_elements[i]->id()
                       <<" from _elements, but couldn't find it")
  }
  _parent_elements.clear();

  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit )
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->remove_parent_children();
    for (unsigned int j = 0; j < curr_elem->num_nodes(); j++)
    {
      EFAnode *curr_node = curr_elem->get_node(j);
      _inverse_connectivity[curr_node].insert(curr_elem);
    }
  }

  for (unsigned int i = 0; i < _permanent_nodes.size(); ++i)
    _permanent_nodes[i]->remove_parent();

  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  _temp_nodes.clear();

  _new_nodes.clear();
  _child_elements.clear();

  //TODO: Sanity check to make sure that there are no nodes that are not connected
  //      to an element -- there shouldn't be any
}

void
ElementFragmentAlgorithm::restoreFragmentInfo(EFAelement * const elem, const EFAelement * const from_elem)
{
  elem->restore_fragment(from_elem);
}

void
ElementFragmentAlgorithm::createChildElements()
{
  //temporary container for new elements -- will be merged with Elements
  std::map<unsigned int, EFAelement*> newChildElements;

  //loop over the original elements in the mesh
  std::map<unsigned int, EFAelement*>::iterator eit;
  std::map<unsigned int, EFAelement*>::iterator ElementsEnd = _elements.end();
  for (eit = _elements.begin(); eit != ElementsEnd; ++eit)
  {
    EFAelement *curr_elem = eit->second;
    curr_elem->create_child(_crack_tip_elements, _elements, newChildElements, 
                            _child_elements, _parent_elements, _temp_nodes);
  } // loop over elements
  //Merge newChildElements back in with Elements
  _elements.insert(newChildElements.begin(),newChildElements.end());
}

void
ElementFragmentAlgorithm::connectFragments(bool mergeUncutVirtualEdges)
{
  //now perform the comparison on the children
  for (unsigned int elem_iter = 0; elem_iter < _child_elements.size(); elem_iter++)
  {
    EFAelement *childElem = _child_elements[elem_iter];
    childElem->connect_neighbors(_permanent_nodes, _embedded_nodes, _temp_nodes,
                                 _inverse_connectivity, mergeUncutVirtualEdges);
  } // loop over child elements

  std::vector<EFAelement*>::iterator vit;
  for (vit = _child_elements.begin(); vit != _child_elements.end(); )
  {
    if (*vit == NULL)
    {
      vit = _child_elements.erase(vit);
    }
    else
    {
      ++vit;
    }
  }
}

void
ElementFragmentAlgorithm::sanityCheck()
{
  //Make sure there are no remaining TempNodes
  if (_temp_nodes.size() > 0)
  {
    std::cout<<"_temp_nodes size > 0.  size="<<_temp_nodes.size()<<std::endl;
    printMesh();
    exit(1);
  }
}

void
ElementFragmentAlgorithm::findCrackTipElements()
{
  std::set<EFAelement*>::iterator sit;
  //Delete all elements that were previously flagged as crack tip elements if they have
  //been split (and hence appear in ParentElements).
  for (unsigned int i = 0; i < _parent_elements.size(); ++i)
  {
    sit = _crack_tip_elements.find(_parent_elements[i]);
    if (sit != _crack_tip_elements.end())
    {
      _crack_tip_elements.erase(sit);
    }
  }

  //Go through new child elements to find elements that are newly at the crack tip due to
  //crack growth.
  for (unsigned int elem_iter = 0; elem_iter < _child_elements.size(); elem_iter++)
  {
    EFAelement *childElem = _child_elements[elem_iter];
    if (childElem->frag_has_tip_edges())
      _crack_tip_elements.insert(childElem);
  } // loop over (new) child elements

  //std::cout<<"Crack tip elements: ";
  //for (sit=CrackTipElements.begin(); sit!=CrackTipElements.end(); ++sit)
  //{
  //  std::cout<<(*sit)->id<<" ";
  //}
  //std::cout<<std::endl;
}

void
ElementFragmentAlgorithm::printMesh()
{
  std::cout<<"============================================================"
           <<"=================================================="<<std::endl;
  std::cout<<"                                            CutElemMesh Data"<<std::endl;
  std::cout<<"============================================================"
           <<"=================================================="<<std::endl;
  std::cout << "Permanent Nodes:" << std::endl;
  std::map<unsigned int, EFAnode*>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit )
    std::cout << "  " << mit->second->id() << std::endl;
  std::cout << "Temp Nodes:" << std::endl;
  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit )
    std::cout << "  " << mit->second->id() << std::endl;
  std::cout << "Embedded Nodes:" << std::endl;
  for (mit = _embedded_nodes.begin(); mit != _embedded_nodes.end(); ++mit )
    std::cout << "  " << mit->second->id() << std::endl;
  std::cout << "Parent Elements:" << std::endl;
  for (unsigned int i = 0; i < _parent_elements.size(); ++i)
    std::cout << " " << _parent_elements[i]->id() << std::endl;
  std::cout << "Child Elements:" << std::endl;
  for (unsigned int i = 0; i < _child_elements.size(); ++i)
    std::cout << " " << _child_elements[i]->id() << std::endl;
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
  for (eit = _elements.begin(); eit != _elements.end(); ++eit )
  {
    EFAelement* currElem = eit->second;
    currElem->print_elem();
  }
}

EFAelement*
ElementFragmentAlgorithm::getElemByID(unsigned int id)
{
  std::map<unsigned int, EFAelement*>::iterator mit = _elements.find(id);
  if (mit == _elements.end())
    CutElemMeshError("in getElemByID() could not find element: "<<id)
  return mit->second;
}

unsigned int
ElementFragmentAlgorithm::getElemIdByNodes(unsigned int * node_id)
{
  unsigned int elem_id = 99999;
  std::map<unsigned int, EFAelement*>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
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
