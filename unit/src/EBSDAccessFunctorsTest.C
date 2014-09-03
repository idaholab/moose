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

#include "EBSDAccessFunctorsTest.h"

//Moose includes
#include "EBSDAccessFunctors.h"

CPPUNIT_TEST_SUITE_REGISTRATION( EBSDAccessFunctorsTest );

void
EBSDAccessFunctorsTest::setUp()
{
}

void
EBSDAccessFunctorsTest::tearDown()
{
}

void
EBSDAccessFunctorsTest::pointData()
{
  EBSDAccessFunctors::EBSDPointData d;
  EBSDAccessFunctors::EBSDPointDataFunctor * v;

  // we initialize this data sctructure with varying data and check if it is read correctly each time
  const double offset = 17.3232;
  for (unsigned int i = 0; i < 10; ++i)
  {
    // initialize data
    d.phi1  = i * offset + 0.4;
    d.phi   = i * offset + 1.5;
    d.phi2  = i * offset + 2.6;
    d.phase = i * offset + 3.7;
    d.symmetry = i * offset + 4.9;
    d.grain = (unsigned int)(i * offset) + 7;
    d.op    = (unsigned int)(i * offset) + 9;

    // check functors
    v = new EBSDAccessFunctors::EBSDPointDataPhi1;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phi1, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDPointDataPhi;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phi, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDPointDataPhi2;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phi2, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDPointDataPhase;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phase, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDPointDataSymmetry;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.symmetry, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDPointDataGrain;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), Real(d.grain), 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDPointDataOp;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), Real(d.op), 1e-9 );
    delete v;
  }
}

void
EBSDAccessFunctorsTest::avgData()
{
  EBSDAccessFunctors::EBSDAvgData d;
  EBSDAccessFunctors::EBSDAvgDataFunctor * v;

  // we initialize this data sctructure with varying data and check if it is read correctly each time
  const double offset = 17.3232;
  for (unsigned int i = 0; i < 10; ++i)
  {
    // initialize data
    d.phi1  = i * offset + 0.4;
    d.phi   = i * offset + 1.5;
    d.phi2  = i * offset + 2.6;
    d.phase = i * offset + 3.7;
    d.symmetry = i * offset + 4.9;

    // check functors
    v = new EBSDAccessFunctors::EBSDAvgDataPhi1;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phi1, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDAvgDataPhi;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phi, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDAvgDataPhi2;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phi2, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDAvgDataPhase;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.phase, 1e-9 );
    delete v;
    v = new EBSDAccessFunctors::EBSDAvgDataSymmetry;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (*v)(d), d.symmetry, 1e-9 );
    delete v;
  }
}
