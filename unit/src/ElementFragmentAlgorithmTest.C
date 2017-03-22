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
#include "ElementFragmentAlgorithmTest.h"

#include "MooseUtils.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ElementFragmentAlgorithmTest);

ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest() {}

ElementFragmentAlgorithmTest::~ElementFragmentAlgorithmTest() {}

void
ElementFragmentAlgorithmTest::CheckNodes(std::map<unsigned int, EFANode *> & nodes,
                                         std::vector<unsigned int> & gold)
{
  std::map<unsigned int, EFANode *>::iterator mit;
  std::vector<unsigned int> test;
  for (mit = nodes.begin(); mit != nodes.end(); ++mit)
    test.push_back(mit->second->id());

  CPPUNIT_ASSERT(test.size() == gold.size());
  for (unsigned int i = 0; i < test.size(); i++)
    CPPUNIT_ASSERT(test[i] == gold[i]);
}

void
ElementFragmentAlgorithmTest::CheckElements(std::vector<EFAElement *> & elems,
                                            std::vector<unsigned int> & gold)
{
  std::vector<EFAElement *>::iterator it;
  std::vector<unsigned int> test;
  for (it = elems.begin(); it != elems.end(); ++it)
    test.push_back((*it)->id());

  CPPUNIT_ASSERT(test.size() == gold.size());
  for (unsigned int i = 0; i < test.size(); i++)
    CPPUNIT_ASSERT(test[i] == gold[i]);
}

void
ElementFragmentAlgorithmTest::CheckElements(std::set<EFAElement *> & elems,
                                            std::set<unsigned int> & gold)
{
  std::set<EFAElement *>::iterator it;
  std::set<unsigned int> test;
  for (it = elems.begin(); it != elems.end(); ++it)
    test.insert((*it)->id());

  CPPUNIT_ASSERT(test.size() == gold.size());

  std::set<unsigned int> intersection;
  set_intersection(test.begin(),
                   test.end(),
                   gold.begin(),
                   gold.end(),
                   std::inserter(intersection, intersection.begin()));

  CPPUNIT_ASSERT(intersection.size() == gold.size());
}

void
ElementFragmentAlgorithmTest::case1Common(ElementFragmentAlgorithm & MyMesh)
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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest1a()
{
  //  Moose::out<<"\n ***** Running case 1a *****"<<std::endl;
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case1Common(MyMesh);

  // creates all new elements (children)
  // creates all new temporary nodes
  // sets the links in the children according to the new temporary nodes
  MyMesh.updateTopology();

  //  MyMesh.printMesh();
  // CPPUNIT_ASSERT(false);

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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest1b()
{
  //  Moose::out<<"\n ***** Running case 1b *****"<<std::endl;
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

  //  MyMesh.printMesh();

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
ElementFragmentAlgorithmTest::case2Mesh(ElementFragmentAlgorithm & MyMesh)
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
ElementFragmentAlgorithmTest::case2Intersections(ElementFragmentAlgorithm & MyMesh)
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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest2a()
{
  //  Moose::out<<"\n ***** Running case 2a *****"<<std::endl;
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case2Mesh(MyMesh);
  case2Intersections(MyMesh);
  MyMesh.updateTopology();
  //  MyMesh.printMesh();

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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest2b()
{
  //  Moose::out<<"\n ***** Running case 2b *****"<<std::endl;
  //  Moose::out<<"\nCut first element:"<<std::endl;
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case2Mesh(MyMesh);

  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 1, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
  //  MyMesh.printMesh();

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

  //  Moose::out<<"\nCut second element:"<<std::endl;
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

  //  Moose::out<<"\nCut second element:"<<std::endl;
  MyMesh.addElemEdgeIntersection((unsigned int)1, 1, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
  //  MyMesh.printMesh();

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

  //  Moose::out<<"\nCut third element:"<<std::endl;
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

  //  Moose::out<<"\nCut third element:"<<std::endl;
  MyMesh.addElemEdgeIntersection((unsigned int)6, 2, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
  //  MyMesh.printMesh();

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

  //  Moose::out<<"\nFinal:"<<std::endl;
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();
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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest3()
{
  //  Moose::out<<"\n ***** Running case 3 *****"<<std::endl;

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

  //  MyMesh.printMesh();

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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest4()
{
  //  Moose::out<<"\n ***** Running case 4 *****"<<std::endl;

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

  //  MyMesh.printMesh();

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
ElementFragmentAlgorithmTest::case5Mesh(ElementFragmentAlgorithm & MyMesh)
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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest5a()
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
  //  Moose::out<<"\n ***** Running case 5a *****"<<std::endl;
  //  Moose::out<<"\nFirst cut:"<<std::endl;
  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 2, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

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
  //  Moose::out<<"\nSecond cut:"<<std::endl;
  MyMesh.addElemEdgeIntersection((unsigned int)4, 1, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)4, 3, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

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
  //  Moose::out<<"\nThird cut:"<<std::endl;
  MyMesh.addFragEdgeIntersection((unsigned int)14, 3, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest5b()
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
  //  Moose::out<<"\n ***** Running case 5b *****"<<std::endl;
  //  Moose::out<<"\nFirst cut:"<<std::endl;
  MyMesh.addElemEdgeIntersection((unsigned int)0, 0, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)0, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)1, 2, 0.5);
  MyMesh.addElemEdgeIntersection((unsigned int)2, 2, 0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

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
  //  Moose::out<<"\nSecond cut:"<<std::endl;
  MyMesh.addElemEdgeIntersection((unsigned int)9, 1, 0.5);
  MyMesh.addFragEdgeIntersection((unsigned int)9, 2, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

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
  //  Moose::out<<"\nThird cut:"<<std::endl;
  MyMesh.addElemEdgeIntersection((unsigned int)12, 1, 0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  //  MyMesh.printMesh();

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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest5c()
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
  //  Moose::out<<"\n ***** Running case 5c *****"<<std::endl;
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
  //  MyMesh.printMesh();

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
ElementFragmentAlgorithmTest::case6Mesh(ElementFragmentAlgorithm & MyMesh)
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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest6a()
{
  //  Moose::out<<"\n ***** Running case 6a *****"<<std::endl;
  ElementFragmentAlgorithm MyMesh(Moose::out);
  case6Mesh(MyMesh);
  //  Moose::out << " ***** original mesh *****" << std::endl;
  //  MyMesh.printMesh();

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
  //  Moose::out << " ***** new mesh *****" << std::endl;
  // second time, just test
  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  //  MyMesh.printMesh();

  // print crack tip elems
  //  Moose::out << " ***** crack tip elements *****" << std::endl;
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();
  //  std::set<EFAElement*>::iterator it;
  //  for (it = crack_tip_elem.begin(); it != crack_tip_elem.end(); ++it)
  //    Moose::out << (*it)->id() << " ";
  //  Moose::out << std::endl;

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

void
ElementFragmentAlgorithmTest::ElementFragmentAlgorithmTest6b()
{
  //  Moose::out<<"\n ***** Running case 6b *****"<<std::endl;
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
  //  MyMesh.printMesh();

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
  /*  MyMesh.updatePhysicalLinksAndFragments();
    MyMesh.updateTopology();
    MyMesh.clearAncestry();
    MyMesh.updateEdgeNeighbors();
    MyMesh.initCrackTipTopology();*/

  //  Moose::out << "***** final mesh *****" << std::endl;
  //  MyMesh.printMesh();

  // print crack tip elems
  //  Moose::out << " ***** crack tip elements *****" << std::endl;
  std::set<EFAElement *> crack_tip_elem = MyMesh.getCrackTipElements();
  //  std::set<EFAElement*>::iterator it;
  //  for (it = crack_tip_elem.begin(); it != crack_tip_elem.end(); ++it)
  //    Moose::out << (*it)->id() << " ";
  //  Moose::out << std::endl;

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
