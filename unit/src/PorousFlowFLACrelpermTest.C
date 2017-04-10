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
#include "gtest/gtest.h"

#include "PorousFlowFLACrelperm.h"

const double eps = 1.0E-8;

TEST(PorousFlowFLACrelperm, relperm)
{
  EXPECT_NEAR(1.0, PorousFlowFLACrelperm::relativePermeability(1.0E30, 2.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowFLACrelperm::relativePermeability(-1.0, 2.7), 1.0E-5);
  EXPECT_NEAR(0.111976072427008, PorousFlowFLACrelperm::relativePermeability(0.3, 2.7), 1.0E-5);
  EXPECT_NEAR(0.208087549965399, PorousFlowFLACrelperm::relativePermeability(0.8, 12.7), 1.0E-5);
}

TEST(PorousFlowFLACrelperm, drelperm)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowFLACrelperm::dRelativePermeability(1.0E30, 2.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowFLACrelperm::dRelativePermeability(-1.0, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::relativePermeability(0.3 + eps, 2.7) -
        PorousFlowFLACrelperm::relativePermeability(0.3, 2.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowFLACrelperm::dRelativePermeability(0.3, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::relativePermeability(0.8 + eps, 0.65) -
        PorousFlowFLACrelperm::relativePermeability(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowFLACrelperm::dRelativePermeability(0.8, 0.65), 1.0E-5);
}

TEST(PorousFlowFLACrelperm, d2relperm)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowFLACrelperm::d2RelativePermeability(1.0E30, 2.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowFLACrelperm::d2RelativePermeability(-1.0, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::dRelativePermeability(0.3 + eps, 2.7) -
        PorousFlowFLACrelperm::dRelativePermeability(0.3, 2.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowFLACrelperm::d2RelativePermeability(0.3, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::dRelativePermeability(0.8 + eps, 0.65) -
        PorousFlowFLACrelperm::dRelativePermeability(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowFLACrelperm::d2RelativePermeability(0.8, 0.65), 1.0E-5);
}
