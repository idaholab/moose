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

#ifndef EBSDACCESSFUNCTORSTEST_H
#define EBSDACCESSFUNCTORSTEST_H

#include "gtest/gtest.h"
#include "EBSDAccessFunctors.h"

class EBSDAccessFunctorsTest : public ::testing::Test, public EBSDAccessFunctors
{
protected:
  void SetUp()
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

  EBSDPointData _point;
  EBSDAvgData _avg;

  EulerAngles _angles;
};

#endif // EBSDACCESSFUNCTORSTEST_H
