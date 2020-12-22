//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBSDAccessFunctorsTest.h"

TEST_F(EBSDAccessFunctorsTest, test)
{
  RealVectorValue reference_angle(0.1, 0.2, 0.3);
  Point reference_point1 = Point(9.0, 10.0, 11.0);
  Point reference_point2 = Point(6.0, 7.0, 8.0);

  // Test point data access
  {
    EBSDPointDataPhi1 phi1;
    EXPECT_EQ(phi1(_point), _point._phi1);
    EBSDPointDataPhi phi;
    EXPECT_EQ(phi(_point), _point._Phi);
    EBSDPointDataPhi2 phi2;
    EXPECT_EQ(phi2(_point), _point._phi2);

    EBSDPointDataPhase phase;
    EXPECT_EQ(phase(_point), _point._phase);
    EBSDPointDataSymmetry symmetry;
    EXPECT_EQ(symmetry(_point), _point._symmetry);
    EBSDPointDataFeatureID feature_id;
    EXPECT_EQ(feature_id(_point), _point._feature_id);

    for (unsigned int i = 0; i < 3; ++i)
    {
      EBSDPointDataCustom custom(i);
      EXPECT_EQ(custom(_point), _point._custom[i]);
    }
  }

  // Test average data access
  {
    RealVectorValue angle = *(_avg._angles);
    EXPECT_EQ((angle - reference_angle).norm(), 0);

    EBSDAvgDataPhi1 phi1;
    EXPECT_EQ(phi1(_avg), angle(0));
    EBSDAvgDataPhi phi;
    EXPECT_EQ(phi(_avg), angle(1));
    EBSDAvgDataPhi2 phi2;
    EXPECT_EQ(phi2(_avg), angle(2));

    EBSDAvgDataPhase phase;
    EXPECT_EQ(phase(_avg), _avg._phase);
    EBSDAvgDataSymmetry symmetry;
    EXPECT_EQ(symmetry(_avg), _avg._symmetry);
    EBSDAvgDataFeatureID feature_id;
    EXPECT_EQ(feature_id(_avg), _avg._feature_id);
    EBSDAvgDataLocalID local;
    EXPECT_EQ(local(_avg), _avg._local_id);

    for (unsigned int i = 0; i < 3; ++i)
    {
      EBSDAvgDataCustom custom(i);
      EXPECT_EQ(custom(_avg), _avg._custom[i]);
    }
  }
}
