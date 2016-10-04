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
#include "libmesh/parallel.h"

#include <cmath>
#include <cstdio>


CPPUNIT_TEST_SUITE_REGISTRATION( StepperTest );

void
updateInfo(StepperInfo* si, double dt)
{
  si->prev_prev_dt = si->prev_dt;
  si->prev_dt = dt;
  si->time += dt;
  si->step_count++;
}

StepperInfo blankInfo()
{
    return {1, 0, 0, 0, "", 0, 0, true, 0, nullptr, nullptr, nullptr, false, 0};
}

void
StepperTest::fixedPoint()
{
  struct testcase {
    std::string title;
    double tol;
    // For the first call to advance, positive causes returned dt to be
    // doubled and used.  Negative causes halved.  Zero means to use the
    // stepper's returned dt.
    int violate_dt;
    // The fixed point times to hit.
    std::vector<double> times;
    // The expected time sequence from the stepper. This sequence should be
    // exactly one longer than the times sequence to check what the stepper does
    // after an extra call to advance.
    std::vector<double> want;
  };

  testcase tests[] = {
    {
      "zero-len-seq",
      1e-10,
      0,
      {},
      {0}
    },{
      "normal-seq",
      1e-10,
      0,
      {1, 2, 5},
      {1, 2, 5, 8}
    },{
      // checks that the stepper doesn't repeat time t0 with a dt=0 if a
      // fixed-point time is equal to the initial time.
      "want-point-t0-on-same",
      1e-10,
      0,
      {0, 1, 3},
      {1, 3, 5, 7}
    },{
      "under-tolerance",
      1e-10,
      0,
      {1e-11, 1, 3},
      {1, 3, 5, 7}
    },{
      "over-tolerance",
      1e-10,
      0,
      {1e-9, 1, 3},
      {1e-9, 1, 3, 5}
    },{
      "violate-dt-under",
      1e-10,
      -1,
      {1, 2, 5},
      {0.5,1, 2, 5}
    },{
      "violate-dt-over",
      1e-10,
      +1,
      {1, 2, 5},
      {2, 5, 8, 11}
    }
  };

  for(int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    double tol = tests[i].tol;
    std::vector<double> times = tests[i].times;
    std::vector<double> want = tests[i].want;
    FixedPointStepper stepper(times, tol);
    StepperInfo si = blankInfo();

    for (int j = 0; j < times.size(); j++)
    {
      dt = stepper.advance(&si);
      if (j == 0 && tests[i].violate_dt > 0)
        dt *= 2;
      else if (j == 0 && tests[i].violate_dt < 0)
        dt /= 2;
      updateInfo(&si, dt);
      if (std::abs(want[j] - si.time) > tol) {
        printf("case %d (%s) failed:\n", i+1, tests[i].title.c_str());
        printf("    time_step %d: want %f, got %f\n", j, want[j], si.time);
        CPPUNIT_ASSERT(false);
      }
    }
    dt = stepper.advance(&si);
    updateInfo(&si, dt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(want[want.size()-1], si.time, tol);
  }
}

void
StepperTest::maxRatio()
{
  struct testcase {
    std::string title;
    double max_ratio;
    // The fixed point times to hit - for underlying FixedPointStepper.
    std::vector<double> times;
    // The expected time sequence from the stepper. This sequence should be
    // exactly one longer than the times sequence to check what the stepper does
    // after an extra call to advance.
    std::vector<double> want;
  };

  testcase tests[] = {
    {
      "constr",
      2,
      {1, 2, 5},
      {1, 2, 4, 5}
    },{
      "no-constr",
      3,
      {1, 2, 5},
      {1, 2, 5, 8}
    }
  };

  double tol = 1e-10;

  for(int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    double max_ratio = tests[i].max_ratio;
    std::vector<double> times = tests[i].times;
    std::vector<double> want = tests[i].want;
    FixedPointStepper s(times, tol);
    MaxRatioStepper stepper(&s, max_ratio);
    StepperInfo si = blankInfo();

    for (int j = 0; j < times.size(); j++)
    {
      dt = stepper.advance(&si);
      updateInfo(&si, dt);
      if (std::abs(want[j] - si.time) > tol) {
        printf("case %d (%s) failed:\n", i+1, tests[i].title.c_str());
        printf("    time_step %d: want %f, got %f\n", j, want[j], si.time);
        CPPUNIT_ASSERT(false);
      }
    }
    dt = stepper.advance(&si);
    updateInfo(&si, dt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(want[want.size()-1], si.time, tol);
  }
}

void
StepperTest::everyN()
{
  struct testcase {
    std::string title;
    int every_n;
    // The fixed point times to hit - for underlying FixedPointStepper.
    std::vector<double> times;
    // The expected time sequence from the stepper. This sequence should be
    // exactly one longer than the times sequence to check what the stepper does
    // after an extra call to advance.
    std::vector<double> want;
  };

  testcase tests[] = {
    {
      "constr",
      2,
      {1, 3, 5},
      {1, 2, 3, 4}
    }
  };

  double tol = 1e-10;

  for(int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    std::vector<double> times = tests[i].times;
    std::vector<double> want = tests[i].want;
    FixedPointStepper s(times, tol);
    EveryNStepper stepper(&s, tests[i].every_n);
    StepperInfo si = blankInfo();

    for (int j = 0; j < times.size(); j++)
    {
      dt = stepper.advance(&si);
      updateInfo(&si, dt);
      if (std::abs(want[j] - si.time) > tol) {
        printf("case %d (%s) failed:\n", i+1, tests[i].title.c_str());
        printf("    time_step %d: want %f, got %f\n", j, want[j], si.time);
        CPPUNIT_ASSERT(false);
      }
    }
    dt = stepper.advance(&si);
    updateInfo(&si, dt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(want[want.size()-1], si.time, tol);
  }
}


void
StepperTest::scratch() {
  libMesh::Parallel::Communicator dummy_comm;

  int n = 5;
  auto tmp = NumericVector<Number>::build(dummy_comm);
  tmp->init(n, n, false, SERIAL);
  NumericVector<Number>& vec = *tmp.get();
  vec.set(0, 1);
  vec.set(1, 2);
  vec.set(2, 5);

  auto tmp2 = tmp->clone();
  NumericVector<Number>& vec2 = *tmp2.get();

  std::cout << vec(0) << "\n";
  std::cout << vec(1) << "\n";
  std::cout << vec(2) << "\n";

  vec += vec2;

  std::cout << vec(0) << "\n";
  std::cout << vec(1) << "\n";
  std::cout << vec(2) << "\n";
}

