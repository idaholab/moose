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

#include "IndirectSortTest.h"

//Moose includes
#include "IndirectSort.h"

CPPUNIT_TEST_SUITE_REGISTRATION( IndirectSort );

void
IndirectSort::realSort()
{
  double a1[] = {1.2, 2.3, 1.4, 5.2, 3.4, 7.5, 1.5, 3.14};
  std::vector<double> a(a1, a1+8);

  std::vector<size_t> b;
  Moose::indirectSortLess(a.begin(), a.end(), b);

  CPPUNIT_ASSERT( b[0] == 0 );
  CPPUNIT_ASSERT( b[1] == 2 );
  CPPUNIT_ASSERT( b[2] == 6 );
  CPPUNIT_ASSERT( b[3] == 1 );
  CPPUNIT_ASSERT( b[4] == 7 );
  CPPUNIT_ASSERT( b[5] == 4 );
  CPPUNIT_ASSERT( b[6] == 3 );
  CPPUNIT_ASSERT( b[7] == 5 );
}

void
IndirectSort::intSort()
{
  const int length = 8;

  std::vector<int> a(length);
  for (unsigned int i=0; i<length; ++i)
    a[i] = length-i-1;

  std::vector<size_t> b;
  Moose::indirectSortLess(a.begin(), a.end(), b);

  CPPUNIT_ASSERT( b[0] == 7 );
  CPPUNIT_ASSERT( b[1] == 6 );
  CPPUNIT_ASSERT( b[2] == 5 );
  CPPUNIT_ASSERT( b[3] == 4 );
  CPPUNIT_ASSERT( b[4] == 3 );
  CPPUNIT_ASSERT( b[5] == 2 );
  CPPUNIT_ASSERT( b[6] == 1 );
  CPPUNIT_ASSERT( b[7] == 0 );
}

void
IndirectSort::testStableSort()
{
  const int length = 4;

  std::vector<std::pair<int, int> > a;
  std::vector<size_t> b;
  a.reserve(length);

  a.push_back(std::make_pair(3, 1));
  a.push_back(std::make_pair(2, 2));
  a.push_back(std::make_pair(2, 3));
  a.push_back(std::make_pair(1, 4));

  Moose::indirectSortLess(a.begin(), a.end(), b);

  CPPUNIT_ASSERT( b[0] == 3 );
  CPPUNIT_ASSERT( b[1] == 1 );
  CPPUNIT_ASSERT( b[2] == 2 );
  CPPUNIT_ASSERT( b[3] == 0 );
}

void
IndirectSort::testDoubleSort()
{
  const int length = 5;

  std::vector<int> a(length);

  a[0] = 154;
  a[1] = 20;
  a[2] = 36;
  a[3] = 4;
  a[4] = 81;

  std::vector<size_t> b;
  Moose::indirectSortGreater(a.begin(), a.end(), b);

  std::vector<size_t> c;
  Moose::indirectSortLess(b.begin(), b.end(), c);

  CPPUNIT_ASSERT( c[0] == 0 );
  CPPUNIT_ASSERT( c[1] == 3 );
  CPPUNIT_ASSERT( c[2] == 2 );
  CPPUNIT_ASSERT( c[3] == 4 );
  CPPUNIT_ASSERT( c[4] == 1 );
}
