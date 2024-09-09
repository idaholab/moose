//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/point.h"

namespace MeshCoarseningUtils
{
/**
 * Utility routine to gather vertex nodes for, and elements contained in, for a coarse QUAD or HEX
 * element
 * @param interior_node the node inside the coarse element
 * @param reference_node another node used to order the vertex nodes in a clockwise order for QUAD
 * elements
 * @param elem an element containing the node. Its neighbor lists must be up to date so it
 *        should come from a prepared mesh
 * @param tentative_coarse_nodes nodes to be used to form the coarse element
 * @param fine_elements fine elements that are inside the coarse element
 */
bool getFineElementsFromInteriorNode(const libMesh::Node & interior_node,
                                     const libMesh::Node & reference_node,
                                     const libMesh::Elem & elem,
                                     std::vector<const libMesh::Node *> & tentative_coarse_nodes,
                                     std::set<const libMesh::Elem *> & fine_elements);

/**
 * Utility routine to re-order a vector of nodes so that they can form a valid quad element
 * @param nodes the vector containing the nodes to re-order
 * @param origin the center of the clock (circle to align nodes around)
 * @param clock_start the start of the clock
 * @param axis the rotation axis (will be normalized)
 */
void reorderNodes(std::vector<const libMesh::Node *> & nodes,
                  const libMesh::Point & origin,
                  const libMesh::Point & clock_start,
                  libMesh::Point & axis);

/**
 * Utility routine to get the index of the node opposite, in the element, to the node
 * of interest
 * @param elem_type type of the element
 * @param node_index local index of the node of interest
 * @return local index of the opposite node, with regards to the element
 */
unsigned int getOppositeNodeIndex(libMesh::ElemType elem_type, unsigned int node_index);
}
