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
#include "CutElemMeshTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CutElemMeshTest );

CutElemMeshTest::CutElemMeshTest()
{
}

CutElemMeshTest::~CutElemMeshTest()
{}

void
CutElemMeshTest::case1Common(ElementFragmentAlgorithm &MyMesh)
{
  // 0 ----- 1 ----- 2
  // |       |       |
  // |       |       |
  // |       |       |
  // 3 ----- 4 ----- 5

  std::vector< std::vector<unsigned int> > quads;
  unsigned int q1[] = {0,3,4,1};
  std::vector<unsigned int> v1 (q1, q1 + sizeof(q1) / sizeof(unsigned int) );
  quads.push_back(v1);
  unsigned int q2[] = {1,4,5,2};
  std::vector<unsigned int> v2 (q2, q2 + sizeof(q2) / sizeof(unsigned int) );
  quads.push_back(v2);

  MyMesh.addElements( quads );

  MyMesh.updateEdgeNeighbors();


  // 0 ----- 1 ----- 2
  // |       |       |
  // x-------x       |
  // |       |       |
  // 3 ----- 4 ----- 5

  //these will create the embedded nodes - elemid, edgeid, location
  MyMesh.addEdgeIntersection((unsigned int) 0,0,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 0,2,0.5);

  //algorithm assumes that an element can only be cut by one crack
  //segment at a time
  MyMesh.updatePhysicalLinksAndFragments();
}

void
CutElemMeshTest::CutElemMeshTest1a()
{
  std::cout<<"\n ***** Running case 1a *****"<<std::endl;
  ElementFragmentAlgorithm MyMesh;
  case1Common(MyMesh);

  //creates all new elements (children)
  //creates all new temporary nodes
  //sets the links in the children according to the new temporary nodes
  MyMesh.updateTopology();

  MyMesh.printMesh();
  //CPPUNIT_ASSERT(false);
}

void
CutElemMeshTest::CutElemMeshTest1b()
{
  std::cout<<"\n ***** Running case 1b *****"<<std::endl;
  ElementFragmentAlgorithm MyMesh;
  case1Common(MyMesh);

  //creates all new elements (children)
  //creates all new temporary nodes
  //sets the links in the children according to the new temporary nodes
  MyMesh.updateTopology();

  //Once mesh has been cut, run it through the cutting algorithm again
  //to test that it can handle the topology at a crack tip.
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();

  MyMesh.updateTopology();

  MyMesh.printMesh();
}

void
CutElemMeshTest::case2Mesh(ElementFragmentAlgorithm &MyMesh)
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

  std::vector< std::vector<unsigned int> > quads;
  unsigned int q1[] = {0,3,4,1};
  std::vector<unsigned int> v1 (q1, q1 + sizeof(q1) / sizeof(unsigned int) );
  quads.push_back(v1);
  unsigned int q2[] = {1,4,5,2};
  std::vector<unsigned int> v2 (q2, q2 + sizeof(q2) / sizeof(unsigned int) );
  quads.push_back(v2);
  unsigned int q3[] = {4,3,6,5};
  std::vector<unsigned int> v3 (q3, q3 + sizeof(q3) / sizeof(unsigned int) );
  quads.push_back(v3);

  MyMesh.addElements( quads );

  MyMesh.updateEdgeNeighbors();
}

void
CutElemMeshTest::case2Intersections(ElementFragmentAlgorithm &MyMesh)
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

  MyMesh.addEdgeIntersection((unsigned int) 0,0,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 0,1,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 1,1,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 1,2,0.5);

  MyMesh.updatePhysicalLinksAndFragments();
}

void
CutElemMeshTest::CutElemMeshTest2a()
{
  std::cout<<"\n ***** Running case 2a *****"<<std::endl;
  ElementFragmentAlgorithm MyMesh;
  case2Mesh(MyMesh);
  case2Intersections(MyMesh);
  MyMesh.updateTopology();
  MyMesh.printMesh();
}

void CutElemMeshTest::CutElemMeshTest2b()
{
  std::cout<<"\n ***** Running case 2b *****"<<std::endl;
  ElementFragmentAlgorithm MyMesh;
  case2Mesh(MyMesh);
  case2Intersections(MyMesh);
  MyMesh.updateTopology(false);
  MyMesh.printMesh();
}

void CutElemMeshTest::CutElemMeshTest2c()
{
  std::cout<<"\n ***** Running case 2c *****"<<std::endl;
  std::cout<<"\nCut first element:"<<std::endl;
  ElementFragmentAlgorithm MyMesh;
  case2Mesh(MyMesh);

  MyMesh.addEdgeIntersection((unsigned int) 0,0,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 0,1,0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
//  MyMesh.printMesh();

//  std::cout<<"\nCut second element:"<<std::endl;
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();

  std::cout<<"\nCut second element:"<<std::endl;
  MyMesh.addEdgeIntersection((unsigned int) 1,1,0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
//  MyMesh.printMesh();

//  std::cout<<"\nCut third element:"<<std::endl;
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors(); 
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();

  std::cout<<"\nCut third element:"<<std::endl;
  MyMesh.addEdgeIntersection((unsigned int) 6,2,0.5);// I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology(false);
//  MyMesh.printMesh();

//  std::cout<<"\nFinal:"<<std::endl;
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();
}

void CutElemMeshTest::CutElemMeshTest3()
{
  std::cout<<"\n ***** Running case 3 *****"<<std::endl;

  // 0 ----- 1 ----- 2
  // |       |       |
  // |       |       |
  // |       |       |
  // 3 ----- 4 ----- 5
  // |       |       |
  // |       |       |
  // |       |       |
  // 6 ----- 7 ----- 8

  std::vector< std::vector<unsigned int> > quads;
  std::vector< std::vector<unsigned int> > quads2;
  unsigned int q1[] = {0,3,4,1};
  std::vector<unsigned int> v1 (q1, q1 + sizeof(q1) / sizeof(unsigned int) );
  quads.push_back(v1);
  unsigned int q2[] = {1,4,5,2};
  std::vector<unsigned int> v2 (q2, q2 + sizeof(q2) / sizeof(unsigned int) );
  quads.push_back(v2);
  unsigned int q3[] = {4,3,6,7};
  std::vector<unsigned int> v3 (q3, q3 + sizeof(q3) / sizeof(unsigned int) );
  quads.push_back(v3);
  unsigned int q4[] = {5,4,7,8};
  std::vector<unsigned int> v4 (q4, q4 + sizeof(q4) / sizeof(unsigned int) );
  quads2.push_back(v4);

  ElementFragmentAlgorithm MyMesh;
  MyMesh.addElements( quads );
  MyMesh.addElements( quads2 ); //do in 2 batches just to test that it works

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

  //these will create the embedded nodes - elemid, edgeid, location
  MyMesh.addEdgeIntersection((unsigned int) 1,1,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 1,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 2,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 2,3,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 3,0,0.5);  //not necessary, but test it
  MyMesh.addEdgeIntersection((unsigned int) 3,1,0.5);  //not necessary, but test it

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();

  MyMesh.printMesh();
}

void CutElemMeshTest::CutElemMeshTest4()
{
  std::cout<<"\n ***** Running case 4 *****"<<std::endl;

  ElementFragmentAlgorithm MyMesh;

  {
    unsigned int qary[] = {0, 1, 2, 3};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,0);
  }

  {
    unsigned int qary[] = {1, 4, 5, 2};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,1);
  }

  {
    unsigned int qary[] = {4, 6, 7, 5};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,2);
  }

  {
    unsigned int qary[] = {6, 8, 9, 7};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,3);
  }

  {
    unsigned int qary[] = {3, 2, 10, 11};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,4);
  }

  {
    unsigned int qary[] = {2, 5, 12, 10};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,5);
  }

  {
    unsigned int qary[] = {5, 7, 13, 12};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,6);
  }

  {
    unsigned int qary[] = {7, 9, 14, 13};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,7);
  }

  {
    unsigned int qary[] = {11, 10, 15, 16};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,8);
  }

  {
    unsigned int qary[] = {10, 12, 17, 15};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,9);
  }

  {
    unsigned int qary[] = {12, 13, 18, 17};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,10);
  }

  {
    unsigned int qary[] = {13, 14, 19, 18};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,11);
  }

  {
    unsigned int qary[] = {16, 15, 20, 21};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,12);
  }

  {
    unsigned int qary[] = {15, 17, 22, 20};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,13);
  }

  {
    unsigned int qary[] = {17, 18, 23, 22};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,14);
  }

  {
    unsigned int qary[] = {18, 19, 24, 23};
    std::vector<unsigned int> qvec (qary, qary + sizeof(qary) / sizeof(unsigned int) );
    MyMesh.addElement(qvec,15);
  }

  MyMesh.updateEdgeNeighbors();

  //these will create the embedded nodes - elemid, edgeid, location
  MyMesh.addEdgeIntersection((unsigned int) 1,1,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 1,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 2,0,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 5,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 8,1,0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();

  MyMesh.printMesh();
}

void
CutElemMeshTest::case5Mesh(ElementFragmentAlgorithm &MyMesh)
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

  std::vector< std::vector<unsigned int> > quads;
  unsigned int q1[] = {0,4,5,1};
  std::vector<unsigned int> v1 (q1, q1 + sizeof(q1) / sizeof(unsigned int) );
  quads.push_back(v1);
  unsigned int q2[] = {1,5,6,2};
  std::vector<unsigned int> v2 (q2, q2 + sizeof(q2) / sizeof(unsigned int) );
  quads.push_back(v2);
  unsigned int q3[] = {2,6,7,3};
  std::vector<unsigned int> v3 (q3, q3 + sizeof(q3) / sizeof(unsigned int) );
  quads.push_back(v3);
  unsigned int q4[] = {4,8,9,5};
  std::vector<unsigned int> v4 (q4, q4 + sizeof(q4) / sizeof(unsigned int) );
  quads.push_back(v4);
  unsigned int q5[] = {5,9,10,6};
  std::vector<unsigned int> v5 (q5, q5 + sizeof(q5) / sizeof(unsigned int) );
  quads.push_back(v5);
  unsigned int q6[] = {6,10,11,7};
  std::vector<unsigned int> v6 (q6, q6 + sizeof(q6) / sizeof(unsigned int) );
  quads.push_back(v6);

  MyMesh.addElements( quads );
  MyMesh.updateEdgeNeighbors();
}

void CutElemMeshTest::CutElemMeshTest5a()
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

  ElementFragmentAlgorithm MyMesh;
  case5Mesh(MyMesh);

  // add the horizontal cut
  std::cout<<"\n ***** Running case 5a *****"<<std::endl;
  std::cout<<"\nFirst cut:"<<std::endl;
  MyMesh.addEdgeIntersection((unsigned int) 0,0,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 0,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 1,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 2,2,0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();

  // add the lower part of the vertical cut
  std::cout<<"\nSecond cut:"<<std::endl;
  MyMesh.addEdgeIntersection((unsigned int) 4,1,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 4,3,0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();

  // add the upper vertical cut
  std::cout<<"\nThird cut:"<<std::endl;
  MyMesh.addFragEdgeIntersection((unsigned int) 14,3,0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();
}

void CutElemMeshTest::CutElemMeshTest5b()
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

  ElementFragmentAlgorithm MyMesh;
  case5Mesh(MyMesh);

  // add the horizontal cut
  std::cout<<"\n ***** Running case 5b *****"<<std::endl;
  std::cout<<"\nFirst cut:"<<std::endl;
  MyMesh.addEdgeIntersection((unsigned int) 0,0,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 0,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 1,2,0.5);
  MyMesh.addEdgeIntersection((unsigned int) 2,2,0.5);

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();

  // add the upper part of the vertical cut
  std::cout<<"\nSecond cut:"<<std::endl;
  MyMesh.addEdgeIntersection((unsigned int) 9,1,0.5);
  MyMesh.addFragEdgeIntersection((unsigned int) 9,2,0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();

  // add the lower vertical cut
  std::cout<<"\nThird cut:"<<std::endl;
  MyMesh.addEdgeIntersection((unsigned int) 12,1,0.5); // I cheated here

  MyMesh.updatePhysicalLinksAndFragments();
  MyMesh.updateTopology();
  MyMesh.clearAncestry();
  MyMesh.updateEdgeNeighbors();
  MyMesh.initCrackTipTopology();
  MyMesh.printMesh();
}
