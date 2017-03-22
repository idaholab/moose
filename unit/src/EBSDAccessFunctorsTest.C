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

// Moose includes
#include "EBSDAccessFunctors.h"

CPPUNIT_TEST_SUITE_REGISTRATION(EBSDAccessFunctorsTest);

void
EBSDAccessFunctorsTest::setUp()
{
  // EBSD point data
  _point._phi1 = 1.0;
  _point._Phi = 2.0;
  _point._phi2 = 3.0;
  _point._symmetry = 4;
  _point._feature_id = 5;
  _point._phase = 6;
  _point._p = Point(9.0, 10.0, 11.0);
  _point._custom.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    _point._custom[i] = i + 12.0;

  // Averaged EBSD data
  _avg._angles = &_angles;
  _avg._phase = 1;
  _avg._local_id = 2;
  _avg._symmetry = 3;
  _avg._feature_id = 4;
  _avg._n = 5;
  _avg._p = Point(6.0, 7.0, 8.0);
  _avg._custom.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    _avg._custom[i] = i + 9.0;

  // initialize Euler angle object
  _angles.phi1 = 0.1;
  _angles.Phi = 0.2;
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
    CPPUNIT_ASSERT(phi1(_point) == _point._phi1);
    EBSDPointDataPhi phi;
    CPPUNIT_ASSERT(phi(_point) == _point._Phi);
    EBSDPointDataPhi2 phi2;
    CPPUNIT_ASSERT(phi2(_point) == _point._phi2);

    EBSDPointDataPhase phase;
    CPPUNIT_ASSERT(phase(_point) == _point._phase);
    EBSDPointDataSymmetry symmetry;
    CPPUNIT_ASSERT(symmetry(_point) == _point._symmetry);
    EBSDPointDataFeatureID feature_id;
    CPPUNIT_ASSERT(feature_id(_point) == _point._feature_id);

    for (unsigned int i = 0; i < 3; ++i)
    {
      EBSDPointDataCustom custom(i);
      CPPUNIT_ASSERT(custom(_point) == _point._custom[i]);
    }
  }

  // Test average data access
  {
    RealVectorValue angle = *(_avg._angles);
    CPPUNIT_ASSERT((angle - reference_angle).size() == 0);

    EBSDAvgDataPhi1 phi1;
    CPPUNIT_ASSERT(phi1(_avg) == angle(0));
    EBSDAvgDataPhi phi;
    CPPUNIT_ASSERT(phi(_avg) == angle(1));
    EBSDAvgDataPhi2 phi2;
    CPPUNIT_ASSERT(phi2(_avg) == angle(2));

    EBSDAvgDataPhase phase;
    CPPUNIT_ASSERT(phase(_avg) == _avg._phase);
    EBSDAvgDataSymmetry symmetry;
    CPPUNIT_ASSERT(symmetry(_avg) == _avg._symmetry);
    EBSDAvgDataFeatureID feature_id;
    CPPUNIT_ASSERT(feature_id(_avg) == _avg._feature_id);
    EBSDAvgDataLocalID local;
    CPPUNIT_ASSERT(local(_avg) == _avg._local_id);

    for (unsigned int i = 0; i < 3; ++i)
    {
      EBSDAvgDataCustom custom(i);
      CPPUNIT_ASSERT(custom(_avg) == _avg._custom[i]);
    }
  }
}
