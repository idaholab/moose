//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"
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

