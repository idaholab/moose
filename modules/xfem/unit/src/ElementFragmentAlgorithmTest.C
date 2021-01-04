//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseUtils.h"
#include "ElementFragmentAlgorithm.h"

// set this global to true to enable a lot of mesh data output to the console
const bool debug_print_mesh = false;

void
CheckNodes(std::map<unsigned int, EFANode *> & nodes, std::vector<unsigned int> & gold)
{
  std::map<unsigned int, EFANode *>::iterator mit;
  std::vector<unsigned int> test;
  for (mit = nodes.begin(); mit != nodes.end(); ++mit)
    test.push_back(mit->second->id());

  ASSERT_EQ(test.size(), gold.size());
  for (unsigned int i = 0; i < test.size(); i++)
    EXPECT_EQ(test[i], gold[i]);
}

void
CheckElements(std::vector<EFAElement *> & elems, std::vector<unsigned int> & gold)
{
  std::vector<EFAElement *>::iterator it;
  std::vector<unsigned int> test;
  for (it = elems.begin(); it != elems.end(); ++it)
    test.push_back((*it)->id());

  ASSERT_EQ(test.size(), gold.size());
  for (unsigned int i = 0; i < test.size(); i++)
    EXPECT_EQ(test[i], gold[i]);
}

void
CheckElements(std::set<EFAElement *> & elems, std::set<unsigned int> & gold)
{
  std::set<EFAElement *>::iterator it;
  std::set<unsigned int> test;
  for (it = elems.begin(); it != elems.end(); ++it)
    test.insert((*it)->id());

  ASSERT_EQ(test.size(), gold.size());

  std::set<unsigned int> intersection;
  set_intersection(test.begin(),
                   test.end(),
                   gold.begin(),
                   gold.end(),
                   std::inserter(intersection, intersection.begin()));

  ASSERT_EQ(intersection.size(), gold.size());
}

void
case1Common(ElementFragmentAlgorithm & MyMesh)
{
  // 0 ----- 1 ----- 2
  // |       |       |
  // |       |       |
  // |       |       |
  // 3 ----- 4 ----- 5

  std::vector<std::vector<unsigned int>> quads;
  std::vector<unsigned int> v1 = {0, 3, 4, 1};
  quads.push_back(v1);
  std::vector<unsigned int> v2 = {1, 4, 5, 2};
  quads.push_back(v2);

  MyMesh.add2DElements(quads);

  MyMesh.updateEdgeNeighbors();

  // 0 ----- 1 ----- 2
  // |       |       |
  // x-------x       |
  // |       |       |
  // 3 ----- 4 ----- 5

  // these will create the embedded nodes - elemid, edgeid, location
  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 2, 0.5);

  // algorithm assumes that an element can only be cut by one crack
  // segment at a time
  MyMesh.updatePhysicalLinksAndFragments();
}

TEST(ElementFragmentAlgorithm, test1a)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case1Common(MyMesh);

  // creates all new elements (children)
  // creates all new temporary nodes
  // sets the links in the children according to the new temporary nodes
  MyMesh.updateTopology();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold = {2, 3, 4};
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold = {0, 1};
  CheckElements(parent_elem, pe_gold);
}

TEST(ElementFragmentAlgorithm, test1b)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case1Common(MyMesh);

  // creates all new elements (children)
  // creates all new temporary nodes
  // sets the links in the children according to the new temporary nodes
  MyMesh.updateTopology();

  // Once mesh has been cut, run it through the cutting algorithm again
  // to test that it can handle the topology at a crack tip.
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  MyMesh.updateTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

void
case2Mesh(ElementFragmentAlgorithm & MyMesh)
{
  // 0 ----- 1 ----- 2
  // |       |       |
  // |       4       |
  // |     /   \     |
  // |   /       \   |
  // | /           \ |
  // 3               5
  //   \           /
  //     \       /
  //       \   /
  //         6

  std::vector<std::vector<unsigned int>> quads;
  std::vector<unsigned int> v1 = {0, 3, 4, 1};
  quads.push_back(v1);
  std::vector<unsigned int> v2 = {1, 4, 5, 2};
  quads.push_back(v2);
  std::vector<unsigned int> v3 = {4, 3, 6, 5};
  quads.push_back(v3);

  MyMesh.add2DElements(quads);

  MyMesh.updateEdgeNeighbors();
}

void
case2Intersections(ElementFragmentAlgorithm & MyMesh)
{
  // 0 ----- 1 ----- 2
  // |       |       |
  // x       4       x
  // | \   /   \   / |
  // |   x ----- x   |
  // | /           \ |
  // 3               5
  //   \           /
  //     \       /
  //       \   /
  //         6

  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 1, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 1, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
}

TEST(ElementFragmentAlgorithm, test2a)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case2Mesh(MyMesh);
  case2Intersections(MyMesh);
  MyMesh.updateTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold = {3, 4, 5, 6, 7, 8};
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold = {0, 1, 2};
  CheckElements(parent_elem, pe_gold);
}

TEST(ElementFragmentAlgorithm, test2b)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case2Mesh(MyMesh);

  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 1, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
  if (debug_print_mesh)
    MyMesh.printMesh();

  {
    // Test permanent nodes
    std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
    std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    CheckNodes(permanent_nodes, pn_gold);

    // Test temp nodes
    std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
    std::vector<unsigned int> tn_gold;
    CheckNodes(temp_nodes, tn_gold);

    // Test embedded nodes
    std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
    std::vector<unsigned int> en_gold = {0, 1};
    CheckNodes(embedded_nodes, en_gold);

    // Test child elements
    std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
    std::vector<unsigned int> ce_gold = {3, 4, 5};
    CheckElements(child_elem, ce_gold);

    // Test parent elements
    std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
    std::vector<unsigned int> pe_gold = {0, 2};
    CheckElements(parent_elem, pe_gold);
  }

  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  MyMesh.addElemEdgeIntersection((unsigned int)1, 1, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
  if (debug_print_mesh)
    MyMesh.printMesh();

  {
    // Test permanent nodes
    std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
    std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CheckNodes(permanent_nodes, pn_gold);

    // Test temp nodes
    std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
    std::vector<unsigned int> tn_gold;
    CheckNodes(temp_nodes, tn_gold);

    // Test embedded nodes
    std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
    std::vector<unsigned int> en_gold = {0, 1, 2};
    CheckNodes(embedded_nodes, en_gold);

    // Test child elements
    std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
    std::vector<unsigned int> ce_gold = {6, 7, 8, 9, 10};
    CheckElements(child_elem, ce_gold);

    // Test parent elements
    std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
    std::vector<unsigned int> pe_gold = {1, 3, 4, 5};
    CheckElements(parent_elem, pe_gold);
  }

  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  MyMesh.addElemEdgeIntersection((unsigned int)6, 2, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
  if (debug_print_mesh)
    MyMesh.printMesh();

  {
    // Test permanent nodes
    std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
    std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    CheckNodes(permanent_nodes, pn_gold);

    // Test temp nodes
    std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
    std::vector<unsigned int> tn_gold;
    CheckNodes(temp_nodes, tn_gold);

    // Test embedded nodes
    std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
    std::vector<unsigned int> en_gold = {0, 1, 2, 3};
    CheckNodes(embedded_nodes, en_gold);

    // Test child elements
    std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
    std::vector<unsigned int> ce_gold = {11, 12, 13, 14, 15};
    CheckElements(child_elem, ce_gold);

    // Test parent elements
    std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
    std::vector<unsigned int> pe_gold = {6, 8, 9, 10};
    CheckElements(parent_elem, pe_gold);
  }

  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();
  {
    // Test permanent nodes
    std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
    std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    CheckNodes(permanent_nodes, pn_gold);

    // Test temp nodes
    std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
    std::vector<unsigned int> tn_gold;
    CheckNodes(temp_nodes, tn_gold);

    // Test embedded nodes
    std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
    std::vector<unsigned int> en_gold = {0, 1, 2, 3};
    CheckNodes(embedded_nodes, en_gold);

    // Test child elements
    std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
    std::vector<unsigned int> ce_gold;
    CheckElements(child_elem, ce_gold);

    // Test parent elements
    std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
    std::vector<unsigned int> pe_gold;
    CheckElements(parent_elem, pe_gold);
  }
}

TEST(ElementFragmentAlgorithm, test3)
{

  // 0 ----- 1 ----- 2
  // |       |       |
  // |       |       |
  // |       |       |
  // 3 ----- 4 ----- 5
  // |       |       |
  // |       |       |
  // |       |       |
  // 6 ----- 7 ----- 8

  std::vector<std::vector<unsigned int>> quads;
  std::vector<std::vector<unsigned int>> quads2;
  std::vector<unsigned int> v1 = {0, 3, 4, 1};
  quads.push_back(v1);
  std::vector<unsigned int> v2 = {1, 4, 5, 2};
  quads.push_back(v2);
  std::vector<unsigned int> v3 = {4, 3, 6, 7};
  quads.push_back(v3);
  std::vector<unsigned int> v4 = {5, 4, 7, 8};
  quads2.push_back(v4);

  ElementFragmentAlgorithm MyMesh(Moose::out);
  MyMesh.add2DElements(quads);
  MyMesh.add2DElements(quads2); // do in 2 batches just to test that it works

  MyMesh.updateEdgeNeighbors();

  // 0 ----- 1 ----- 2
  // |       |       |
  // |       |       x
  // |       |     / |
  // 3 ----- 4 --x-- 5
  // |       | /     |
  // |       x       |
  // |     / |       |
  // 6 --x-- 7 ----- 8

  // these will create the embedded nodes - elemid, edgeid, location
  MyMesh.addElemEdgeIntersection((unsigned int)1, 1, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 3, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)3, 0, 0.5); // not necessary, but test it
  MyMesh.addElemEdgeIntersection((unsigned int)3, 1, 0.5); // not necessary, but test it

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold = {4, 5, 6, 7, 8, 9};
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold = {1, 2, 3};
  CheckElements(parent_elem, pe_gold);
}

TEST(ElementFragmentAlgorithm, test4)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);

  {
    std::vector<unsigned int> qvec = {0, 1, 2, 3};
    MyMesh.add2DElement(qvec, 0);
  }

  {
    std::vector<unsigned int> qvec = {1, 4, 5, 2};
    MyMesh.add2DElement(qvec, 1);
  }

  {
    std::vector<unsigned int> qvec = {4, 6, 7, 5};
    MyMesh.add2DElement(qvec, 2);
  }

  {
    std::vector<unsigned int> qvec = {6, 8, 9, 7};
    MyMesh.add2DElement(qvec, 3);
  }

  {
    std::vector<unsigned int> qvec = {3, 2, 10, 11};
    MyMesh.add2DElement(qvec, 4);
  }

  {
    std::vector<unsigned int> qvec = {2, 5, 12, 10};
    MyMesh.add2DElement(qvec, 5);
  }

  {
    std::vector<unsigned int> qvec = {5, 7, 13, 12};
    MyMesh.add2DElement(qvec, 6);
  }

  {
    std::vector<unsigned int> qvec = {7, 9, 14, 13};
    MyMesh.add2DElement(qvec, 7);
  }

  {
    std::vector<unsigned int> qvec = {11, 10, 15, 16};
    MyMesh.add2DElement(qvec, 8);
  }

  {
    std::vector<unsigned int> qvec = {10, 12, 17, 15};
    MyMesh.add2DElement(qvec, 9);
  }

  {
    std::vector<unsigned int> qvec = {12, 13, 18, 17};
    MyMesh.add2DElement(qvec, 10);
  }

  {
    std::vector<unsigned int> qvec = {13, 14, 19, 18};
    MyMesh.add2DElement(qvec, 11);
  }

  {
    std::vector<unsigned int> qvec = {16, 15, 20, 21};
    MyMesh.add2DElement(qvec, 12);
  }

  {
    std::vector<unsigned int> qvec = {15, 17, 22, 20};
    MyMesh.add2DElement(qvec, 13);
  }

  {
    std::vector<unsigned int> qvec = {17, 18, 23, 22};
    MyMesh.add2DElement(qvec, 14);
  }

  {
    std::vector<unsigned int> qvec = {18, 19, 24, 23};
    MyMesh.add2DElement(qvec, 15);
  }

  MyMesh.updateEdgeNeighbors();

  // these will create the embedded nodes - elemid, edgeid, location
  MyMesh.addElemEdgeIntersection((unsigned int)1, 1, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)5, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)8, 1, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                       22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold = {16, 17, 18, 19, 20, 21, 22, 23, 24};
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold = {1, 2, 5, 8, 9};
  CheckElements(parent_elem, pe_gold);
}

void
case5Mesh(ElementFragmentAlgorithm & MyMesh)
{
  // 0 ----- 1 ----- 2 ----- 3
  // |       |       |       |
  // |       |       |       |
  // |       |       |       |
  // 4 ----- 5 ----- 6 ----- 7
  // |       |       |       |
  // |       |       |       |
  // |       |       |       |
  // 8 ----- 9 -----10 -----11

  std::vector<std::vector<unsigned int>> quads;
  std::vector<unsigned int> v1 = {0, 4, 5, 1};
  quads.push_back(v1);
  std::vector<unsigned int> v2 = {1, 5, 6, 2};
  quads.push_back(v2);
  std::vector<unsigned int> v3 = {2, 6, 7, 3};
  quads.push_back(v3);
  std::vector<unsigned int> v4 = {4, 8, 9, 5};
  quads.push_back(v4);
  std::vector<unsigned int> v5 = {5, 9, 10, 6};
  quads.push_back(v5);
  std::vector<unsigned int> v6 = {6, 10, 11, 7};
  quads.push_back(v6);

  MyMesh.add2DElements(quads);
  MyMesh.updateEdgeNeighbors();
}

TEST(ElementFragmentAlgorithm, test5a)
{
  // 0 ----- 1 ----- 2 ----- 3
  // |       |       |       |
  // x ----- x --x-- x ----- x
  // |       |   |   |       |
  // 4 ----- 5 --x-- 6 ----- 7
  // |       |   |   |       |
  // |       |   |   |       |
  // |       |   |   |       |
  // 8 ----- 9 --x--10 -----11

  ElementFragmentAlgorithm MyMesh(Moose::out);
  case5Mesh(MyMesh);

  // add the horizontal cut
  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 2, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                                       10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // add the lower part of the vertical cut
  MyMesh.addElemEdgeIntersection((unsigned int)4, 1, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)4, 3, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes2 = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold2 = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
  CheckNodes(permanent_nodes2, pn_gold2);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes2 = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold2;
  CheckNodes(temp_nodes2, tn_gold2);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes2 = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold2 = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes2, en_gold2);

  // Test child elements
  std::vector<EFAElement *> child_elem2 = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold2;
  CheckElements(child_elem2, ce_gold2);

  // Test parent elements
  std::vector<EFAElement *> parent_elem2 = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold2;
  CheckElements(parent_elem2, pe_gold2);

  // add the upper vertical cut
  MyMesh.addFragEdgeIntersection((unsigned int)14, 4, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes3 = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold3 = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
  CheckNodes(permanent_nodes3, pn_gold3);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes3 = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold3;
  CheckNodes(temp_nodes3, tn_gold3);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes3 = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold3 = {0, 1, 2, 3, 4, 5, 6};
  CheckNodes(embedded_nodes3, en_gold3);

  // Test child elements
  std::vector<EFAElement *> child_elem3 = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold3;
  CheckElements(child_elem3, ce_gold3);

  // Test parent elements
  std::vector<EFAElement *> parent_elem3 = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold3;
  CheckElements(parent_elem3, pe_gold3);
}

TEST(ElementFragmentAlgorithm, test5b)
{
  // 0 ----- 1 ----- 2 ----- 3
  // |       |       |       |
  // x ----- x --x-- x ----- x
  // |       |   |   |       |
  // 4 ----- 5 --x-- 6 ----- 7
  // |       |   |   |       |
  // |       |   |   |       |
  // |       |   |   |       |
  // 8 ----- 9 --x--10 -----11

  ElementFragmentAlgorithm MyMesh(Moose::out);
  case5Mesh(MyMesh);

  // add the horizontal cut
  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 2, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                                       10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // add the upper part of the vertical cut
  MyMesh.addElemEdgeIntersection((unsigned int)9, 1, 0.5);
  MyMesh.addFragEdgeIntersection((unsigned int)9, 3, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes2 = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold2 = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
  CheckNodes(permanent_nodes2, pn_gold2);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes2 = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold2;
  CheckNodes(temp_nodes2, tn_gold2);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes2 = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold2 = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes2, en_gold2);

  // Test child elements
  std::vector<EFAElement *> child_elem2 = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold2;
  CheckElements(child_elem2, ce_gold2);

  // Test parent elements
  std::vector<EFAElement *> parent_elem2 = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold2;
  CheckElements(parent_elem2, pe_gold2);

  // add the lower vertical cut
  MyMesh.addElemEdgeIntersection((unsigned int)12, 1, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes3 = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold3 = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
  CheckNodes(permanent_nodes3, pn_gold3);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes3 = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold3;
  CheckNodes(temp_nodes3, tn_gold3);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes3 = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold3 = {0, 1, 2, 3, 4, 5, 6};
  CheckNodes(embedded_nodes3, en_gold3);

  // Test child elements
  std::vector<EFAElement *> child_elem3 = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold3;
  CheckElements(child_elem3, ce_gold3);

  // Test parent elements
  std::vector<EFAElement *> parent_elem3 = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold3;
  CheckElements(parent_elem3, pe_gold3);
}

TEST(ElementFragmentAlgorithm, test5c)
{
  // 0 ----- 1 ----- 2 ----- 3
  // |       |       |       |
  // x ----- x --x-- x ----- x
  // |       |   |   |       |
  // 4 ----- 5 --x-- 6 ----- 7
  // |       |   |   |       |
  // |       |   |   |       |
  // |       |   |   |       |
  // 8 ----- 9 --x--10 -----11

  ElementFragmentAlgorithm MyMesh(Moose::out);
  case5Mesh(MyMesh);

  // add the horizontal cut
  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 1, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)4, 1, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

void
case6Mesh(ElementFragmentAlgorithm & MyMesh)
{
  // 3D test
  std::vector<unsigned int> v1 = {0, 1, 4, 3, 9, 10, 13, 12};
  MyMesh.add3DElement(v1, 0);

  std::vector<unsigned int> v2 = {1, 2, 5, 4, 10, 11, 14, 13};
  MyMesh.add3DElement(v2, 1);

  std::vector<unsigned int> v3 = {3, 4, 7, 6, 12, 13, 16, 15};
  MyMesh.add3DElement(v3, 2);

  std::vector<unsigned int> v4 = {4, 5, 8, 7, 13, 14, 17, 16};
  MyMesh.add3DElement(v4, 3);

  std::vector<unsigned int> v5 = {9, 10, 13, 12, 18, 19, 22, 21};
  MyMesh.add3DElement(v5, 4);

  std::vector<unsigned int> v6 = {10, 11, 14, 13, 19, 20, 23, 22};
  MyMesh.add3DElement(v6, 5);

  std::vector<unsigned int> v7 = {12, 13, 16, 15, 21, 22, 25, 24};
  MyMesh.add3DElement(v7, 6);

  std::vector<unsigned int> v8 = {13, 14, 17, 16, 22, 23, 26, 25};
  MyMesh.add3DElement(v8, 7);

  MyMesh.updateEdgeNeighbors();
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|---------|-\-|---------|-| |
   | |O O O O O O O O          | |\|
   | |*|---------|-|O|---------|-|-|
   | |*|         | |O|         | | |
   | |*|         | |O|         | | |
   |-|*|---------|-|O|---------| | |
    \|*|          \|O|          \| |
     \*|-----------\O|-----------\ |
      * * * * * * * O|            \|
       \-------------\-------------\
*/

TEST(ElementFragmentAlgorithm, test6a)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);

  MyMesh.addElemFaceIntersection(1, 4, cut_edge_id, cut_position); // unnecessary, just test
  MyMesh.addElemFaceIntersection(2, 1, cut_edge_id, cut_position); // unnecessary, just test

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  // second time, just test
  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {10, 11};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |*
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | * |
   |-|-|---------|-|-|---------| | |
   |\| |         |*| |         |\| |
   | |-|---------|-\-|-----*---|-| |
   | | |         | |\|         | |*|
   | | |---------|-|-|---------|-|-|
   | | |         | * |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | *
    *| |          \| |          \* |
     \-|----*------\-|---------*-\ |
      \|            *|       *    \|
       \-------------\-----*-------\
*/

TEST(ElementFragmentAlgorithm, test6b)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(1, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(1, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(3, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(4, 2, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(5, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(5, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(5, 4, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(7, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(7, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(7, 2, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                       26, 27, 28, 29, 30, 32, 33, 34, 35, 36, 37};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold = {0, 1, 3, 4, 5, 7};
  CheckElements(parent_elem, pe_gold);

  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  // second time cut, just test

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes2 = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold2 = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                        26, 27, 28, 29, 30, 32, 33, 34, 35, 36, 37};
  CheckNodes(permanent_nodes2, pn_gold2);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes2 = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold2;
  CheckNodes(temp_nodes2, tn_gold2);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes2 = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold2 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  CheckNodes(embedded_nodes2, en_gold2);

  // Test child elements
  std::vector<EFAElement *> child_elem2 = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold2;
  CheckElements(child_elem2, ce_gold2);

  // Test parent elements
  std::vector<EFAElement *> parent_elem2 = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold2;
  CheckElements(parent_elem2, pe_gold2);

  // Test crack tip elements
  unsigned int cte[] = {14, 17};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|---------|-\-|---------|-| |
   | |\|         | |\|         | |\|
   | | |---------|-|-|---------|-|-|
   | | |         | | |         | | |
   | O O O O O O O O |         | | |
   |-|*|---------| |O|---------| | |
    \| * * * * * * * O          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6c)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {10, 11};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|----O----|-\-|---------|-| |
   | |\|    OO   | |\|         | |\|
   | | |----O-*--|-|-|---------|-|-|
   | | |    O *  | | |         | | |
   | | |    O *  | | |         | | |
   |-|-|----O-*--|-|-|---------| | |
    \| |    O *   \| |          \| |
     \-|----O-*----\-|-----------\ |
      \|     **     \|            \|
       \------*------\--------------
*/

TEST(ElementFragmentAlgorithm, test6d)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {10, 11};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|---O-----|-\-|---------|-| |
   | |\| O  O    | |\|         | |\|
   | | O-----O---|-|-|---------|-|-|
   | O |   *     | | |         | | |
   | |*| *       | | |         | | |
   |-|-*---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6e)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {10, 11};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | O |         | | |
   | | |         O |O|         | | |
   |-|-|-------O-|-|-*---------| | |
   |\| |     O   |\* |         |\| |
   | |-|---O-----*-\-|---------|-| |
   | |\| O     * | |\|         | |\|
   | | O-----*---|-|-|---------|-|-|
   | O |   *     | | |         | | |
   | |*| *       | | |         | | |
   |-|-*---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6f)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(2, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                       22, 23, 24, 25, 26, 27, 29, 30, 31};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {12, 13, 14};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 3);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----O-----|-\
   | |\          | |\  O  *    | |\
   | | \---------|-|-O-----*---|-|-\
   | | |         | O |   *     | | |
   | | |         O | | *       | | |
   |-|-|-------O-|-|-*---------| | |
   |\| |     O   |\* |         |\| |
   | \-|---O-----*-\-|---------|-\ |
   | |\| O     * | |\|         | |\|
   | | O-----*---|-|-|---------|-|-|
   | O |   *     | | |         | | |
   | |*| *       | | |         | | |
   |-|-*---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6g)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(2, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(3, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                       12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                       24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {14, 15, 16};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 3);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|---------|-\-|---------|-| |
   | |\|         | |\|         | |\|
   | | |---------|-|-|---------|-|-|
   | | |         | | |         | | |
   | o o o o o o o o o o o o o o o |
   |-|*|---------| |*|---------| |*|
    \| * * * * * * * * * * * * *\* *
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6h)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(1, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(1, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                       22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {12, 13};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |*|* * * * * * * * * * * *| |*
   | | \---------|-|*\---------|-|-\
   | |*|         | |*|         | |*|
   | | |         | | |         | | |
   |-|*|---------|-|*|---------| |*|
   |\| |         |\| |         |\| |
   | |*|---------|-\*|---------|-|*|
   | |*|* * * * *|*|*|* * * * *|*|\|
   | |*|---------|-|*|---------|-|*|
   | | |         | | |         | | |
   | |*|         | |*|         | |*|
   |-|-|---------|-|-|---------| | |
    \|*|          \|*|          \|*|
     \-|-----------\-|-----------\ |
      *|* * * * * * *|* * * * * * *|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6i)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);

  MyMesh.addElemFaceIntersection(1, 1, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(1, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(1, 3, cut_edge_id, cut_position);

  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(2, 3, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(2, 4, cut_edge_id, cut_position);

  MyMesh.addElemFaceIntersection(3, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(3, 3, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  // second time, just test
  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                       30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \------*------\-------------\
   |\            |\            |\
   | \----*-*----|-\-----------|-\
   | |\          | |\          | |\
   | | \--*---*--|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |  *   *  | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |  *   *  |\| |         |\| |
   | |-|---------|-\-|---------|-| |
   | |\|  *   *  | |\|         | |\|
   | | |---------|-|-|---------|-|-|
   | | |  *   *  | | |         | | |
   | | |         | | |         | | |
   |-|-|--*---*--|-|-|---------| | |
    \| |          \| |          \| |
     \-|---*--*----\-|-----------\ |
      \|            \|            \|
       \------*------\--------------
*/

TEST(ElementFragmentAlgorithm, test6j)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(6, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(6, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(6, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(6, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                       30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|-o-------|-|-|---------| | |
   |\| *  o      |\| |         |\| |
   | *-|---o-----|-\-|---------|-| |
   * |\|    o    | |\|         | |\|
   |*| |-----o---|-|-|---------|-|-|
   | * |   *     | | |         | | |
   | |*| *       | | |         | | |
   |-|-*---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6k)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);

  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                       22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {10, 13};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |*          | |\          | |\
   | | \-----*---|-|-\---------|-|-\
   | |*|         | | |         | | |
   | | |     *   | | |         | | |
   |-|*|---------|-|-|---------| | |
   |\| |     *   |\| |         |\| |
   | |*|---------|-\-|---------|-| |
   | |\|     *   | |\|         | |\|
   | |*|---------|-|-|---------|-|-|
   | | |     *   | | |         | | |
   | |*|         | | |         | | |
   |-|-|-----*---|-|-|---------| | |
    \|*|          \| |          \| |
     \-|-----*-----\-|-----------\ |
      *|            \|            \|
       \-----*-------\--------------
*/

TEST(ElementFragmentAlgorithm, test6l)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(2, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 4, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                       26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|---------|-\-|---------|-| |
   | |O|         | |\|         | |\|
   | | |----O----|-|-|---------|-|-|
   | | | *       | | |         | | |
   | | *         | | |         | | |
   |-|-|---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6m)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                       22, 23, 24, 25, 26, 27, 28, 29, 30};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {10};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 1);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-------------|-\
   | |\          O |\          | |\
   | | \-------*-|-|-----------|-|-\
   | | |     *   | O |         | | |
   | | |   *     * | |         | | |
   |-|-|-*-----*-|-|-O---------| | |
   |\| *     *   |\* |         |\| |
   | *-|---*-----*-\-|---------|-\ |
   * |\| *     * | |\|         | |\|
   |*| *-----*---|-|-|---------|-|-|
   | * |   *     | | |         | | |
   | |*| *       | | |         | | |
   |-|-*---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6n)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);

  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(2, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(6, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(6, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 5, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 4, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                       26, 27, 28, 31, 32, 33, 34, 35, 36, 37, 39, 40, 41};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {12, 17};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 2);
  CheckElements(crack_tip_elem, cte_gold);
}

/* \-------------\-----*-------\
   |\            |\  *  *      |\
   | \-----------|-*-----*-----|-\
   | |\          * |\  *  *    | |\
   | | \-------*-|-|-*-----*---|-|-\
   | | |     *   | * |   *     | | |
   | | |   *     * | | *       | | |
   |-|-|-*-----*-|-|-*---------| | |
   |\| *     *   |\* |         |\| |
   | *-|---*-----*-\-|---------|-\ |
   * |\| *     * | |\|         | |\|
   |*| *-----*---|-|-|---------|-|-|
   | * |   *     | | |         | | |
   | |*| *       | | |         | | |
   |-|-*---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6o)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);

  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 4, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(2, 5, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;

  MyMesh.addElemFaceIntersection(6, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(6, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(3, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(7, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(7, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                       26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                                       39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|---------|-\-|---------|-| |
   | |\|         | |\|         | |\|
   | | |---------|-|-|---------|-|-|
   | | |         | | |         | | |
   | | |         | | |         | | |
   |-|-|---------|-|-|---------| | |
    \| *          \| |          \| |
     \-|-----------\-|-----------\ |
      *|            \|            \|
       \-----*-------\--------------
*/

TEST(ElementFragmentAlgorithm, test6p)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                       12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                       24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |O          | |\
   | | \---------|-|-\------*--|-|-\
   | | |         | |O|         | | |
   | | |         | | |      *  | | |
   |-|-|---------|-|O|---------| | |
   | | |         |\| |      *  |\| |
   | |-|---------|-\O|---------|-| |
   | |\|         | |\|      *  | |\|
   | | |---------|-|O|---------|-|-|
   | | |         | | |      *  | | |
   | | |         | |O|         | | |
   |-|-|---------|-|-|------*--| | |
    \| |          \|O|          \| |
     \-|-----------\-|------*----\ |
      \|            O|            \|
       \-------------\------*-------
*/

TEST(ElementFragmentAlgorithm, test6q)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(1, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 3, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                       22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \------O----|-\-----------|-\
   | |\          | |*          | |\
   | | \----O----|-|-\------*--|-|-\
   | | |         | |*|         | | |
   | | |    O    | | |      *  | | |
   |-|-|---------|-|*|---------| | |
   | | |    O    |\| |      *  |\| |
   | |-|---------|-\*|---------|-| |
   | |\|    O    | |\|      *  | |\|
   | | |---------|-|*|---------|-|-|
   | | |    O    | | |      *  | | |
   | | |         | |*|         | | |
   |-|-|----O----|-|-|------*--| | |
    \| |          \|*|          \| |
     \-|----O------\-|------*----\ |
      \|            *|            \|
       \-------------\------*-------
*/

TEST(ElementFragmentAlgorithm, test6r)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(1, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 3, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                       26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |*            |\            |\
   |*\------*----|-\-----------|-\
   | |\          | |*          | |\
   |*| \----*----|-|-\------*--|-|-\
   | | |         | |*|         | | |
   |*| |    *    | | |      *  | | |
   |-|-|---------|-|*|---------| | |
   |*| |    *    |\| |      *  |\| |
   | |-|---------|-\*|---------|-| |
   |*|\|    *    | |\|      *  | |\|
   | | |---------|-|*|---------|-|-|
   |*| |    *    | | |      *  | | |
   | | |         | |*|         | | |
   |*|-|----*----|-|-|------*--| | |
    *| |          \|*|          \| |
     \-|----*------\-|------*----\ |
      \|            *|            \|
       \-------------\------*-------
*/

TEST(ElementFragmentAlgorithm, test6s)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(1, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(4, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 4, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(6, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(6, 4, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                       26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                                       39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |*            |\            |\
   | \------*----|-\-----------|-\
   | |\          | |*          | |\
   | | \---------|-|-\-----*---|-|-\
   | | |         | | |   *     | | |
   | * |         | | | *       | | |
   |-|-|---------|-|-*---------| | |
   |\| |         |\| |         |\| |
   | |-|---------|-\-|---------|-| |
   | |*|      *  | |\|         | |\|
   | | |----*----|-|-|---------|-|-|
   | | | *       | | |         | | |
   | | *         | | |         | | |
   |-|-|---------|-|-|---------| | |
    \| |          \| |          \| |
     \-|-----------\-|-----------\ |
      \|            \|            \|
       \-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test6t)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 3, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 4, cut_edge_id, cut_position);
  //
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 0, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(2, 2, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(2, 4, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(2, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 0, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(6, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(6, 4, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                       12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                       24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                                       36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);
}

/* \-------------\-------------\
   |\            |\            |\
   | \-----------|-\-----------|-\
   | |\          | |\          | |\
   | | \---------|-|-\---------|-|-\
   | | |         | | |         | | |
   | | |         | | |         | O |
   |-|-|---------|-|-|---------| | |
   |\| |         |\| |         |\| |
   | |-|---------|-\-|------O--|-| |
   | |\|         | |\|   O     | |*|
   | | |---------|-|-|-O-------|-|-|
   | | |         | | |         | | |
   | | |         | O |         | | |
   |-|-|---------|-|-|---------| | |
    \| |          \| |          \| *
     \-|------O----\-|-----------\ |
      \|            *|            \|
       \-------------\--------*-----
*/

TEST(ElementFragmentAlgorithm, test6u)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(0, 1, cut_edge_id, cut_position);
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(0, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(0, 5, cut_edge_id, cut_position);
  //
  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 0, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 1, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(1, 2, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 3, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(1, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 2;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 2, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 1;
  MyMesh.addElemFaceIntersection(3, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                       12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                       24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5, 6, 7};
  CheckNodes(embedded_nodes, en_gold);

  // Test child elements
  std::vector<EFAElement *> child_elem = MyMesh.getChildElements();
  std::vector<unsigned int> ce_gold;
  CheckElements(child_elem, ce_gold);

  // Test parent elements
  std::vector<EFAElement *> parent_elem = MyMesh.getParentElements();
  std::vector<unsigned int> pe_gold;
  CheckElements(parent_elem, pe_gold);

  // Test crack tip elements
  unsigned int cte[] = {14, 15, 16};
  std::set<unsigned int> cte_gold;
  cte_gold.insert(cte, cte + 3);
  CheckElements(crack_tip_elem, cte_gold);
}

// create new Mesh

/* \-------------\-------------\-------------\
   |\            |\            |\            |\
   | \-----------|-\-----------|-\-----------|-\
   | |           | |           | |           | |
   | |           | |           | |           | |
   |-|-----------|-|-----------| |---------- | |
   |\|           |\|           |\|           |\|
   | |-----------|-\-----------|-|-----------| |
   | |           | |           | |           | |
   | |           | |           | |           | |
   | |           | |           | |           | |
   | |           | |           | |           | |
   | |           | |           | |           | |
   |-|-----------|-|-----------| |---------- | |
   |\|           |\|           |\|           |\|
   | |-----------|-\-----------|-|-----------| |
   | |           | |           | |           | |
   | |           | |           | |           | |
   |-|-----------|-|-----------|-|-----------| |
    \|            \|            \|            \|
     \-------------\-------------\--------------
*/

void
case7Mesh(ElementFragmentAlgorithm & MyMesh)
{
  // 3D test
  std::vector<unsigned int> v1 = {0, 1, 5, 4, 16, 17, 21, 20};
  MyMesh.add3DElement(v1, 0);

  std::vector<unsigned int> v2 = {1, 2, 6, 5, 17, 18, 22, 21};
  MyMesh.add3DElement(v2, 1);

  std::vector<unsigned int> v3 = {2, 3, 7, 6, 18, 19, 23, 22};
  MyMesh.add3DElement(v3, 2);

  std::vector<unsigned int> v4 = {4, 5, 9, 8, 20, 21, 25, 24};
  MyMesh.add3DElement(v4, 3);

  std::vector<unsigned int> v5 = {5, 6, 10, 9, 21, 22, 26, 25};
  MyMesh.add3DElement(v5, 4);

  std::vector<unsigned int> v6 = {6, 7, 11, 10, 22, 23, 27, 26};
  MyMesh.add3DElement(v6, 5);

  std::vector<unsigned int> v7 = {8, 9, 13, 12, 24, 25, 29, 28};
  MyMesh.add3DElement(v7, 6);

  std::vector<unsigned int> v8 = {9, 10, 14, 13, 25, 26, 30, 29};
  MyMesh.add3DElement(v8, 7);

  std::vector<unsigned int> v9 = {10, 11, 15, 14, 26, 27, 31, 30};
  MyMesh.add3DElement(v9, 8);

  MyMesh.updateEdgeNeighbors();
}

/* \-------------\-------------\-------------\
   |\            |\            |\            |\
   | \-----------|-\-----------|-\-----------|-\
   | |           | |           | |           | |
   | |           | |           | |           | |
   |-|-----------|-|-----------| |---------- | |
   |\|           |\|           |\|           |\|
   | |-----------|-\-----------|-|-----------| |
   | |           | |           | |           | |
   * * * * * * * * * * * * * * O |           | |
   |*|           | |           |O|           | |
   | * * * * * * * * * * * * * * O           | |
   | |           | |           | |           | |
   |-|-----------|-|-----------| |---------- | |
   |\|           |\|           |\|           |\|
   | |-----------|-\-----------|-|-----------| |
   | |           | |           | |           | |
   | |           | |           | |           | |
   |-|-----------|-|-----------|-|-----------| |
    \|            \|            \|            \|
     \-------------\-------------\--------------
*/

TEST(ElementFragmentAlgorithm, test7a)
{
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case7Mesh(MyMesh);
  if (debug_print_mesh)
    MyMesh.printMesh();

  std::vector<unsigned int> cut_edge_id(2, 0);
  std::vector<double> cut_position(2, 0.5);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(3, 0, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(3, 2, cut_edge_id, cut_position);
  MyMesh.addElemFaceIntersection(3, 4, cut_edge_id, cut_position);

  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(3, 5, cut_edge_id, cut_position);

  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 0, cut_edge_id, cut_position);
  cut_edge_id[0] = 0;
  cut_edge_id[1] = 2;
  MyMesh.addElemFaceIntersection(4, 2, cut_edge_id, cut_position);
  cut_edge_id[0] = 1;
  cut_edge_id[1] = 3;
  MyMesh.addElemFaceIntersection(4, 5, cut_edge_id, cut_position);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  if (debug_print_mesh)
    MyMesh.printMesh();

  // print crack tip elems
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();

  // Test permanent nodes
  std::map<unsigned int, EFANode *> permanent_nodes = MyMesh.getPermanentNodes();
  std::vector<unsigned int> pn_gold = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                                       14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
                                       28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39};
  CheckNodes(permanent_nodes, pn_gold);

  // Test temp nodes
  std::map<unsigned int, EFANode *> temp_nodes = MyMesh.getTempNodes();
  std::vector<unsigned int> tn_gold;
  CheckNodes(temp_nodes, tn_gold);

  // Test embedded nodes
  std::map<unsigned int, EFANode *> embedded_nodes = MyMesh.getEmbeddedNodes();
  std::vector<unsigned int> en_gold = {0, 1, 2, 3, 4, 5};
  CheckNodes(embedded_nodes, en_gold);
}
