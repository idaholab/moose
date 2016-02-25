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
  // EBSD point data
  _point.phi1 = 1.0;
  _point.Phi = 2.0;
  _point.phi2 = 3.0;
  _point.symmetry = 4;
  _point.grain = 5;
  _point.phase = 6;
  _point.op = 7;
  _point.global = 8;
  _point.p = Point(9.0, 10.0, 11.0);
  _point.custom.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    _point.custom[i] = i + 12.0;

  // Averaged EBSD data
  _avg.angles = &_angles;
  _avg.phase = 1;
  _avg.local = 2;
  _avg.symmetry = 3;
  _avg.grain = 4;
  _avg.n = 5;
  _avg.p = Point(6.0, 7.0, 8.0);
  _avg.custom.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    _avg.custom[i] = i + 9.0;

  // initialize Euler angle object
  _angles.phi1 = 0.1;
  _angles.Phi  = 0.2;
  _angles.phi2 = 0.3;
}

void
EBSDAccessFunctorsTest::tearDown()
{
}

void
EBSDAccessFunctorsTest::test()
{
  RealVectorValue reference_angle(0.1, 0.2, 0.3);
  Point reference_point1 = Point(9.0, 10.0, 11.0);
  Point reference_point2 = Point(6.0, 7.0, 8.0);

  // Test point data access
  {
    EBSDPointDataPhi1 phi1;
    CPPUNIT_ASSERT( phi1(_point) == _point.phi1 );
    EBSDPointDataPhi phi;
    CPPUNIT_ASSERT( phi(_point) == _point.Phi );
    EBSDPointDataPhi2 phi2;
    CPPUNIT_ASSERT( phi2(_point) == _point.phi2 );

    EBSDPointDataPhase phase;
    CPPUNIT_ASSERT( phase(_point) == _point.phase );
    EBSDPointDataSymmetry symmetry;
    CPPUNIT_ASSERT( symmetry(_point) == _point.symmetry );
    EBSDPointDataGrain grain;
    CPPUNIT_ASSERT( grain(_point) == _point.grain );
    EBSDPointDataOp op;
    CPPUNIT_ASSERT( op(_point) == _point.op );

    for (unsigned int i = 0; i < 3; ++i)
    {
      EBSDPointDataCustom custom(i);
      CPPUNIT_ASSERT( custom(_point) == _point.custom[i] );
    }
  }

  // Test average data access
  {
    RealVectorValue angle = *(_avg.angles);
    CPPUNIT_ASSERT( (angle - reference_angle).size() == 0 );

    EBSDAvgDataPhi1 phi1;
    CPPUNIT_ASSERT( phi1(_avg) == angle(0) );
    EBSDAvgDataPhi phi;
    CPPUNIT_ASSERT( phi(_avg) == angle(1) );
    EBSDAvgDataPhi2 phi2;
    CPPUNIT_ASSERT( phi2(_avg) == angle(2) );

    EBSDAvgDataPhase phase;
    CPPUNIT_ASSERT( phase(_avg) == _avg.phase );
    EBSDAvgDataSymmetry symmetry;
    CPPUNIT_ASSERT( symmetry(_avg) == _avg.symmetry );
    EBSDAvgDataGrain grain;
    CPPUNIT_ASSERT( grain(_avg) == _avg.grain );
    EBSDAvgDataLocalID local;
    CPPUNIT_ASSERT( local(_avg) == _avg.local );

    for (unsigned int i = 0; i < 3; ++i)
    {
      EBSDAvgDataCustom custom(i);
      CPPUNIT_ASSERT( custom(_avg) == _avg.custom[i] );
    }
  }
}
