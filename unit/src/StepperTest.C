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

#include "StepperTest.h"

#include "Stepper.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( StepperTest );

void
StepperTest::fixedPoint_1()
{
  double dt = 0;
  double tol = 1e-10;
  std::vector<double> times = {1, 2, 5};
  FixedPointStepper stepper(times, tol);
  StepperInfo si = {0, 0, 0, 0, true};

  for (int i = 0; i < times.size(); i++)
  {
    dt = stepper.advance(&si);
    updateInfo(&si, dt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(si.time, times[i], tol);
  }
}

void
StepperTest::updateInfo(StepperInfo* si, double dt) {
  si->prev_dt = dt;
  si->time += dt;
}

