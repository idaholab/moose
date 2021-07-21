//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// TODO:
// Clean up error checking in (!found_edge)
// Save fragment for uncut element ahead of crack tip to avoid renumbering if only embedded node
// Add common code to compare neighbors & fragments (replace multiple set_intersection calls)

// Handle cases other than 0 or 2 cut edges/elem (include data structure to link cut edges with
// cracks?)
// Allow for more than one cut on an edge
// Support 2d higher order elements
// 3D propagation
// 3D branching

#include "ElementFragmentAlgorithm.h"

#include "EFANode.h"
#include "EFAElement3D.h"
#include "EFAElement2D.h"
#include "EFAFuncs.h"
#include "EFAError.h"

ElementFragmentAlgorithm::ElementFragmentAlgorithm(std::ostream & os) : _ostream(os) {}

ElementFragmentAlgorithm::~ElementFragmentAlgorithm()
{
  std::map<unsigned int, EFANode *>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit)
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = _embedded_nodes.begin(); mit != _embedded_nodes.end(); ++mit)
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = _embedded_permanent_nodes.begin(); mit != _embedded_permanent_nodes.end(); ++mit)
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit)
  {
    delete mit->second;
    mit->second = NULL;
  }
  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    delete eit->second;
    eit->second = NULL;
  }
}

unsigned int
ElementFragmentAlgorithm::add2DElements(std::vector<std::vector<unsigned int>> & quads)
{
  unsigned int first_id = 0;
  unsigned int num_nodes = quads[0].size();

  if (quads.size() == 0)
    EFAError("add2DElements called with empty vector of quads");

  for (unsigned int i = 0; i < quads.size(); ++i)
  {
    unsigned int new_elem_id = Efa::getNewID(_elements);
    EFAElement2D * newElem = new EFAElement2D(new_elem_id, num_nodes);
    _elements.insert(std::make_pair(new_elem_id, newElem));

    if (i == 0)
      first_id = new_elem_id;

    for (unsigned int j = 0; j < num_nodes; ++j)
    {
      EFANode * currNode = NULL;
      std::map<unsigned int, EFANode *>::iterator mit = _permanent_nodes.find(quads[i][j]);
      if (mit == _permanent_nodes.end())
      {
        currNode = new EFANode(quads[i][j], EFANode::N_CATEGORY_PERMANENT);
        _permanent_nodes.insert(std::make_pair(quads[i][j], currNode));
      }
      else
        currNode = mit->second;

      newElem->setNode(j, currNode);
      _inverse_connectivity[currNode].insert(newElem);
    }
    newElem->createEdges();
  }
  return first_id;
}

EFAElement *
ElementFragmentAlgorithm::add2DElement(std::vector<unsigned int> quad, unsigned int id)
{
  unsigned int num_nodes = quad.size();

  std::map<unsigned int, EFAElement *>::iterator mit = _elements.find(id);
  if (mit != _elements.end())
    EFAError("In add2DElement element with id: ", id, " already exists");

  EFAElement2D * newElem = new EFAElement2D(id, num_nodes);
  _elements.insert(std::make_pair(id, newElem));

  for (unsigned int j = 0; j < num_nodes; ++j)
  {
    EFANode * currNode = NULL;
    std::map<unsigned int, EFANode *>::iterator mit = _permanent_nodes.find(quad[j]);
    if (mit == _permanent_nodes.end())
    {
      currNode = new EFANode(quad[j], EFANode::N_CATEGORY_PERMANENT);
      _permanent_nodes.insert(std::make_pair(quad[j], currNode));
    }
    else
      currNode = mit->second;

    newElem->setNode(j, currNode);
    _inverse_connectivity[currNode].insert(newElem);
  }
  newElem->createEdges();
  return newElem;
}

EFAElement *
ElementFragmentAlgorithm::add3DElement(std::vector<unsigned int> quad, unsigned int id)
{
  unsigned int num_nodes = quad.size();
  unsigned int num_faces = 0;
  if (num_nodes == 27)
    num_faces = 6;
  else if (num_nodes == 20)
    num_faces = 6;
  else if (num_nodes == 8)
    num_faces = 6;
  else if (num_nodes == 4)
    num_faces = 4;
  else if (num_nodes == 10)
    num_faces = 4;
  else
    EFAError("In add3DElement element with id: ", id, " has invalid num_nodes");

  std::map<unsigned int, EFAElement *>::iterator mit = _elements.find(id);
  if (mit != _elements.end())
    EFAError("In add3DElement element with id: ", id, " already exists");

  EFAElement3D * newElem = new EFAElement3D(id, num_nodes, num_faces);
  _elements.insert(std::make_pair(id, newElem));

  for (unsigned int j = 0; j < num_nodes; ++j)
  {
    EFANode * currNode = NULL;
    std::map<unsigned int, EFANode *>::iterator mit = _permanent_nodes.find(quad[j]);
    if (mit == _permanent_nodes.end())
    {
      currNode = new EFANode(quad[j], EFANode::N_CATEGORY_PERMANENT);
      _permanent_nodes.insert(std::make_pair(quad[j], currNode));
    }
    else
      currNode = mit->second;

    newElem->setNode(j, currNode);
    _inverse_connectivity[currNode].insert(newElem);
  }
  newElem->createFaces();
  return newElem;
}

void
ElementFragmentAlgorithm::updateEdgeNeighbors()
{
  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * elem = eit->second;
    elem->clearNeighbors();
  }

  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * curr_elem = eit->second;
    curr_elem->setupNeighbors(_inverse_connectivity);
  } // loop over all elements

  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * curr_elem = eit->second;
    curr_elem->neighborSanityCheck();
  }
}

void
ElementFragmentAlgorithm::initCrackTipTopology()
{
  _crack_tip_elements.clear(); // re-build CrackTipElements!
  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * curr_elem = eit->second;
    curr_elem->initCrackTip(_crack_tip_elements); // CrackTipElements changed here
  }
}

void
ElementFragmentAlgorithm::addElemEdgeIntersection(unsigned int elemid,
                                                  unsigned int edgeid,
                                                  double position)
{
  // this method is called when we are marking cut edges
  std::map<unsigned int, EFAElement *>::iterator eit = _elements.find(elemid);
  if (eit == _elements.end())
    EFAError("Could not find element with id: ", elemid, " in addEdgeIntersection");

  EFAElement2D * curr_elem = dynamic_cast<EFAElement2D *>(eit->second);
  if (!curr_elem)
    EFAError("addElemEdgeIntersection: elem ", elemid, " is not of type EFAelement2D");
  curr_elem->addEdgeCut(edgeid, position, NULL, _embedded_nodes, true);
}

void
ElementFragmentAlgorithm::addElemNodeIntersection(unsigned int elemid, unsigned int nodeid)
{
  // this method is called when we are marking cut nodes
  std::map<unsigned int, EFAElement *>::iterator eit = _elements.find(elemid);
  if (eit == _elements.end())
    EFAError("Could not find element with id: ", elemid, " in addElemNodeIntersection");

  EFAElement2D * curr_elem = dynamic_cast<EFAElement2D *>(eit->second);
  if (!curr_elem)
    EFAError("addElemNodeIntersection: elem ", elemid, " is not of type EFAelement2D");

  // Only add cut node when the curr_elem does not have any fragment
  if (curr_elem->numFragments() == 0)
    curr_elem->addNodeCut(nodeid, NULL, _permanent_nodes, _embedded_permanent_nodes);
}

bool
ElementFragmentAlgorithm::addFragEdgeIntersection(unsigned int elemid,
                                                  unsigned int frag_edge_id,
                                                  double position)
{
  // N.B. this method must be called after addEdgeIntersection
  std::map<unsigned int, EFAElement *>::iterator eit = _elements.find(elemid);
  if (eit == _elements.end())
    EFAError("Could not find element with id: ", elemid, " in addFragEdgeIntersection");

  EFAElement2D * elem = dynamic_cast<EFAElement2D *>(eit->second);
  if (!elem)
    EFAError("addFragEdgeIntersection: elem ", elemid, " is not of type EFAelement2D");
  return elem->addFragmentEdgeCut(frag_edge_id, position, _embedded_nodes);
}

void
ElementFragmentAlgorithm::addElemFaceIntersection(unsigned int elemid,
                                                  unsigned int faceid,
                                                  std::vector<unsigned int> edgeid,
                                                  std::vector<double> position)
{
  // this method is called when we are marking cut edges
  std::map<unsigned int, EFAElement *>::iterator eit = _elements.find(elemid);
  if (eit == _elements.end())
    EFAError("Could not find element with id: ", elemid, " in addEdgeIntersection");

  EFAElement3D * curr_elem = dynamic_cast<EFAElement3D *>(eit->second);
  if (!curr_elem)
    EFAError("addElemEdgeIntersection: elem ", elemid, " is not of type EFAelement2D");

  // add cuts to two face edges at the same time
  curr_elem->addFaceEdgeCut(faceid, edgeid[0], position[0], NULL, _embedded_nodes, true, true);
  curr_elem->addFaceEdgeCut(faceid, edgeid[1], position[1], NULL, _embedded_nodes, true, true);
}

void
ElementFragmentAlgorithm::addFragFaceIntersection(unsigned int /*ElemID*/,
                                                  unsigned int /*FragFaceID*/,
                                                  std::vector<unsigned int> /*FragFaceEdgeID*/,
                                                  std::vector<double> /*position*/)
{
  // TODO: need to finish this for 3D problems
}

void
ElementFragmentAlgorithm::updatePhysicalLinksAndFragments()
{
  // loop over the elements in the mesh
  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * curr_elem = eit->second;
    curr_elem->updateFragments(_crack_tip_elements, _embedded_nodes);
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

  unsigned int first_new_node_id = Efa::getNewID(_permanent_nodes);

  createChildElements();
  connectFragments(mergeUncutVirtualEdges);
  sanityCheck();
  updateCrackTipElements();

  std::map<unsigned int, EFANode *>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit)
  {
    if (mit->first >= first_new_node_id)
      _new_nodes.push_back(mit->second);
  }
  clearPotentialIsolatedNodes(); // _new_nodes and _permanent_nodes may change here
}

void
ElementFragmentAlgorithm::reset()
{
  _new_nodes.clear();
  _child_elements.clear();
  _parent_elements.clear();
  _crack_tip_elements.clear();
  _inverse_connectivity.clear();

  std::map<unsigned int, EFANode *>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit)
  {
    delete mit->second;
    mit->second = NULL;
  }
  _permanent_nodes.clear();

  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit)
  {
    delete mit->second;
    mit->second = NULL;
  }
  _temp_nodes.clear();
  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
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
    if (!Efa::deleteFromMap(_elements, _parent_elements[i]))
      EFAError("Attempted to delete parent element: ",
               _parent_elements[i]->id(),
               " from _elements, but couldn't find it");
  }
  _parent_elements.clear();

  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * curr_elem = eit->second;
    curr_elem->clearParentAndChildren();
    for (unsigned int j = 0; j < curr_elem->numNodes(); j++)
    {
      EFANode * curr_node = curr_elem->getNode(j);
      _inverse_connectivity[curr_node].insert(curr_elem);
    }
  }

  std::map<unsigned int, EFANode *>::iterator mit;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit)
    mit->second->removeParent();

  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit)
  {
    delete mit->second;
    mit->second = NULL;
  }
  _temp_nodes.clear();

  _new_nodes.clear();
  _child_elements.clear();

  // TODO: Sanity check to make sure that there are no nodes that are not connected
  //      to an element -- there shouldn't be any
}

void
ElementFragmentAlgorithm::restoreFragmentInfo(EFAElement * const elem,
                                              const EFAElement * const from_elem)
{
  elem->restoreFragment(from_elem);
}

void
ElementFragmentAlgorithm::createChildElements()
{
  // temporary container for new elements -- will be merged with Elements
  std::map<unsigned int, EFAElement *> newChildElements;

  // loop over the original elements in the mesh
  std::map<unsigned int, EFAElement *>::iterator eit;
  std::map<unsigned int, EFAElement *>::iterator ElementsEnd = _elements.end();
  for (eit = _elements.begin(); eit != ElementsEnd; ++eit)
  {
    EFAElement * curr_elem = eit->second;
    curr_elem->createChild(_crack_tip_elements,
                           _elements,
                           newChildElements,
                           _child_elements,
                           _parent_elements,
                           _temp_nodes);
  } // loop over elements
  // Merge newChildElements back in with Elements
  _elements.insert(newChildElements.begin(), newChildElements.end());
}

void
ElementFragmentAlgorithm::connectFragments(bool mergeUncutVirtualEdges)
{
  // now perform the comparison on the children
  for (unsigned int elem_iter = 0; elem_iter < _child_elements.size(); elem_iter++)
  {
    EFAElement * childElem = _child_elements[elem_iter];
    childElem->connectNeighbors(
        _permanent_nodes, _temp_nodes, _inverse_connectivity, mergeUncutVirtualEdges);
    childElem->updateFragmentNode();
  } // loop over child elements

  std::vector<EFAElement *>::iterator vit;
  for (vit = _child_elements.begin(); vit != _child_elements.end();)
  {
    if (*vit == NULL)
      vit = _child_elements.erase(vit);
    else
      ++vit;
  }
}

void
ElementFragmentAlgorithm::sanityCheck()
{
  // Make sure there are no remaining TempNodes
  if (_temp_nodes.size() > 0)
  {
    _ostream << "_temp_nodes size > 0.  size=" << _temp_nodes.size() << std::endl;
    printMesh();
    throw std::runtime_error("_temp_nodes size > 0");
  }
}

void
ElementFragmentAlgorithm::updateCrackTipElements()
{
  std::set<EFAElement *>::iterator sit;
  // Delete all elements that were previously flagged as crack tip elements if they have
  // been split (and hence appear in ParentElements).
  for (unsigned int i = 0; i < _parent_elements.size(); ++i)
  {
    sit = _crack_tip_elements.find(_parent_elements[i]);
    if (sit != _crack_tip_elements.end())
      _crack_tip_elements.erase(sit);
  }

  // Go through new child elements to find elements that are newly at the crack tip due to
  // crack growth.
  for (unsigned int elem_iter = 0; elem_iter < _child_elements.size(); elem_iter++)
  {
    EFAElement * childElem = _child_elements[elem_iter];
    if (childElem->isCrackTipElement())
      _crack_tip_elements.insert(childElem);
  } // loop over (new) child elements

  //_ostream << "Crack tip elements: ";
  // for (sit=CrackTipElements.begin(); sit!=CrackTipElements.end(); ++sit)
  //{
  //  _ostream << (*sit)->id<<" ";
  //}
  //_ostream << std::endl;
}

void
ElementFragmentAlgorithm::printMesh()
{
  _ostream << "============================================================"
           << "==================================================" << std::endl;
  _ostream << "                                            CutElemMesh Data" << std::endl;
  _ostream << "============================================================"
           << "==================================================" << std::endl;
  _ostream << "Permanent Nodes:" << std::endl;
  std::map<unsigned int, EFANode *>::iterator mit;
  unsigned int counter = 0;
  for (mit = _permanent_nodes.begin(); mit != _permanent_nodes.end(); ++mit)
  {
    _ostream << "  " << mit->second->id();
    counter += 1;
    if (counter % 10 == 0)
      _ostream << std::endl;
  }
  _ostream << std::endl;
  _ostream << "Temp Nodes:" << std::endl;
  counter = 0;
  for (mit = _temp_nodes.begin(); mit != _temp_nodes.end(); ++mit)
  {
    _ostream << "  " << mit->second->id();
    counter += 1;
    if (counter % 10 == 0)
      _ostream << std::endl;
  }
  _ostream << std::endl;
  _ostream << "Embedded Nodes:" << std::endl;
  counter = 0;
  for (mit = _embedded_nodes.begin(); mit != _embedded_nodes.end(); ++mit)
  {
    _ostream << "  " << mit->second->id();
    counter += 1;
    if (counter % 10 == 0)
      _ostream << std::endl;
  }
  _ostream << std::endl;
  _ostream << "Embedded Permanent Nodes:" << std::endl;
  counter = 0;
  for (mit = _embedded_permanent_nodes.begin(); mit != _embedded_permanent_nodes.end(); ++mit)
  {
    _ostream << "  " << mit->second->id();
    counter += 1;
    if (counter % 10 == 0)
      _ostream << std::endl;
  }
  _ostream << std::endl;
  _ostream << "Parent Elements:" << std::endl;
  counter = 0;
  for (unsigned int i = 0; i < _parent_elements.size(); ++i)
  {
    _ostream << " " << _parent_elements[i]->id();
    counter += 1;
    if (counter % 10 == 0)
      _ostream << std::endl;
  }
  _ostream << std::endl;
  _ostream << "Child Elements:" << std::endl;
  counter = 0;
  for (unsigned int i = 0; i < _child_elements.size(); ++i)
  {
    _ostream << " " << _child_elements[i]->id();
    counter += 1;
    if (counter % 10 == 0)
      _ostream << std::endl;
  }
  _ostream << std::endl;
  _ostream << "Elements:" << std::endl;
  _ostream << "  id "
           << "|  nodes                "
           << "|  embedded nodes       "
           << "|  edge neighbors       "
           << "|  frag "
           << "|  frag link      ...   " << std::endl;
  _ostream << "------------------------------------------------------------"
           << "--------------------------------------------------" << std::endl;
  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * currElem = eit->second;
    currElem->printElement(_ostream);
  }
}

EFAElement *
ElementFragmentAlgorithm::getElemByID(unsigned int id)
{
  std::map<unsigned int, EFAElement *>::iterator mit = _elements.find(id);
  if (mit == _elements.end())
    EFAError("in getElemByID() could not find element: ", id);
  return mit->second;
}

unsigned int
ElementFragmentAlgorithm::getElemIdByNodes(unsigned int * node_id)
{
  unsigned int elem_id = std::numeric_limits<unsigned int>::max();
  std::map<unsigned int, EFAElement *>::iterator eit;
  for (eit = _elements.begin(); eit != _elements.end(); ++eit)
  {
    EFAElement * curr_elem = eit->second;
    unsigned int counter = 0;
    for (unsigned int i = 0; i < curr_elem->numNodes(); ++i)
    {
      if (curr_elem->getNode(i)->id() == node_id[i])
        counter += 1;
    }
    if (counter == curr_elem->numNodes())
    {
      elem_id = curr_elem->id();
      break;
    }
  }
  return elem_id;
}

void
ElementFragmentAlgorithm::clearPotentialIsolatedNodes()
{
  // Collect all parent nodes that will be isolated
  std::map<EFANode *, std::vector<EFANode *>> isolate_parent_to_child;
  for (unsigned int i = 0; i < _new_nodes.size(); ++i)
  {
    EFANode * parent_node = _new_nodes[i]->parent();
    if (!parent_node)
      EFAError("a new permanent node must have a parent node!");
    bool isParentNodeInNewElem = false;
    for (unsigned int j = 0; j < _child_elements.size(); ++j)
    {
      if (_child_elements[j]->containsNode(parent_node))
      {
        isParentNodeInNewElem = true;
        break;
      }
    }
    if (!isParentNodeInNewElem)
      isolate_parent_to_child[parent_node].push_back(_new_nodes[i]);
  }

  // For each isolated parent node, pick one of its child new node
  // Then, switch that child with its parent for all new elems
  std::map<EFANode *, std::vector<EFANode *>>::iterator mit;
  for (mit = isolate_parent_to_child.begin(); mit != isolate_parent_to_child.end(); ++mit)
  {
    EFANode * parent_node = mit->first;
    EFANode * child_node = (mit->second)[0]; // need to discard it and swap it back to its parent
    for (unsigned int i = 0; i < _child_elements.size(); ++i)
    {
      if (_child_elements[i]->containsNode(child_node))
        _child_elements[i]->switchNode(parent_node, child_node, true);
    }
    _new_nodes.erase(std::remove(_new_nodes.begin(), _new_nodes.end(), child_node),
                     _new_nodes.end());
    Efa::deleteFromMap(_permanent_nodes, child_node);
  }
}
