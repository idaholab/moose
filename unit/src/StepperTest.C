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

struct fixedpointtest {
  double tol;
  // the fixed point times to hit
  std::vector<double> times;
  // the expected time sequence from the stepper. This sequence should be
  // exactly one longer than the times sequence to check what the stepper does
  // after an extra call to advance
  std::vector<double> want;
  StepperInfo initial;
};

void
StepperTest::fixedPoint_1()
{

  fixedpointtest tests[] = {
    {
      1e-10,
      {1, 2, 5},
      {1, 2, 5, 8},
      {0,0,0,0,true},
    },{
      1e-10,
      {0, 1, 3},
      {1, 3, 5, 7},
      {0,0,0,0,true},
    }
  };

  for(int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    double tol = tests[i].tol;
    std::vector<double> times = tests[i].times;
    std::vector<double> want = tests[i].want;
    FixedPointStepper stepper(times, tol);
    StepperInfo si = tests[i].initial;

    for (int j = 0; j < times.size(); j++)
    {
      dt = stepper.advance(&si);
      updateInfo(&si, dt);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(want[j], si.time, tol);
    }
    dt = stepper.advance(&si);
    updateInfo(&si, dt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(want[want.size()-1], si.time, tol);
  }
}

void
StepperTest::updateInfo(StepperInfo* si, double dt) {
  si->prev_dt = dt;
  si->time += dt;
}

