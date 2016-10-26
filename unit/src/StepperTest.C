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
#include "DynStepper.h"

#include <cmath>
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( StepperTest );

void cloneStepperInfo(StepperInfo* src, StepperInfo* dst) {
  dst->step_count = src->step_count;
  dst->time = src->time;
  dst->prev_dt = src->prev_dt;
  dst->prev_prev_dt = src->prev_prev_dt;
  dst->prev_prev_prev_dt = src->prev_prev_prev_dt;
  dst->time_integrator = src->time_integrator;
  dst->nonlin_iters = src->nonlin_iters;
  dst->lin_iters = src->lin_iters;
  dst->prev_converged = src->prev_converged;
  dst->prev_solve_time_secs = src->prev_solve_time_secs;
  dst->prev_prev_solve_time_secs = src->prev_prev_solve_time_secs;
  dst->prev_prev_prev_solve_time_secs = src->prev_prev_prev_solve_time_secs;
  if (src->soln_nonlin)
    dst->soln_nonlin.reset(src->soln_nonlin->clone().release());
  if (src->soln_aux)
    dst->soln_aux.reset(src->soln_aux->clone().release());
  if (src->soln_predicted)
    dst->soln_predicted.reset(src->soln_predicted->clone().release());
};

void
updateInfo(StepperInfo * si, StepperFeedback * sf, double dt, std::map<double, StepperInfo>* snaps = nullptr)
{
  if (sf && sf->rewind) {
    printf("[REWINDING...]\n");
    cloneStepperInfo(&(*snaps)[sf->rewind_time], si);
    return;
  } else if (sf && snaps && sf->snapshot) {
    printf("[SNAPSHOTTING...]\n");
    cloneStepperInfo(si, &(*snaps)[si->time]);
  }

  si->prev_prev_dt = si->prev_dt;
  si->prev_dt = dt;
  si->time += dt;
  si->step_count++;
}

StepperInfo blankInfo()
{
    return {1, 0, 0, 0, 0, "", 0, 0, true, true, 0, 0, 0, nullptr, nullptr, nullptr};
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

  double inf = std::numeric_limits<double>::infinity();

  testcase tests[] = {
    {
      "zero-len-seq",
      1e-10,
      0,
      {},
      {inf}
    },{
      "normal-seq",
      1e-10,
      0,
      {1, 2, 5},
      {1, 2, 5, inf}
    },{
      // checks that the stepper doesn't repeat time t0 with a dt=0 if a
      // fixed-point time is equal to the initial time.
      "want-point-t0-on-same",
      1e-10,
      0,
      {0, 1, 3},
      {1, 3, inf, inf}
    },{
      "under-tolerance",
      1e-10,
      0,
      {1e-11, 1, 3},
      {1, 3, inf, inf}
    },{
      "over-tolerance",
      1e-10,
      0,
      {1e-9, 1, 3},
      {1e-9, 1, 3, inf}
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
      {2, 5, inf, inf}
    }
  };

  for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    double tol = tests[i].tol;
    std::vector<double> times = tests[i].times;
    std::vector<double> want = tests[i].want;
    FixedPointStepper stepper(times, tol);
    StepperInfo si = blankInfo();

    for (int j = 0; j < times.size(); j++)
    {
      dt = stepper.advance(&si, nullptr);
      if (j == 0 && tests[i].violate_dt > 0)
        dt *= 2;
      else if (j == 0 && tests[i].violate_dt < 0)
        dt /= 2;
      updateInfo(&si, nullptr, dt);
      if (std::abs(want[j] - si.time) > tol)
      {
        printf("case %d (%s) failed:\n", i+1, tests[i].title.c_str());
        printf("    time_step %d: want %f, got %f\n", j+1, want[j], si.time);
        CPPUNIT_ASSERT(false);
      }
    }
    dt = stepper.advance(&si, nullptr);
    updateInfo(&si, nullptr, dt);
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
      {1, 2, 5, 14}
    }
  };

  double tol = 1e-10;

  for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    double max_ratio = tests[i].max_ratio;
    std::vector<double> times = tests[i].times;
    std::vector<double> want = tests[i].want;
    Stepper * s = new FixedPointStepper(times, tol);
    MaxRatioStepper stepper(s, max_ratio);
    StepperInfo si = blankInfo();

    for (int j = 0; j < times.size(); j++)
    {
      dt = stepper.advance(&si, nullptr);
      updateInfo(&si, nullptr, dt);
      if (std::abs(want[j] - si.time) > tol)
      {
        printf("case %d (%s) failed:\n", i+1, tests[i].title.c_str());
        printf("    time_step %d: want %f, got %f\n", j+1, want[j], si.time);
        CPPUNIT_ASSERT(false);
      }
    }
    dt = stepper.advance(&si, nullptr);
    updateInfo(&si, nullptr, dt);
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

  for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    std::vector<double> times = tests[i].times;
    std::vector<double> want = tests[i].want;
    Stepper * s = new FixedPointStepper(times, tol);
    EveryNStepper stepper(s, tests[i].every_n);
    StepperInfo si = blankInfo();

    for (int j = 0; j < times.size(); j++)
    {
      dt = stepper.advance(&si, nullptr);
      updateInfo(&si, nullptr, dt);
      if (std::abs(want[j] - si.time) > tol)
      {
        printf("case %d (%s) failed:\n", i+1, tests[i].title.c_str());
        printf("    time_step %d: want %f, got %f\n", j+1, want[j], si.time);
        CPPUNIT_ASSERT(false);
      }
    }
    dt = stepper.advance(&si, nullptr);
    updateInfo(&si, nullptr, dt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(want[want.size()-1], si.time, tol);
  }
}

void
StepperTest::DT2()
{
  struct testcase {
    std::string title;
    double e_tol;
    double e_max;
    std::vector<std::vector<double> > solns;
    std::vector<double> want_times;
    std::vector<double> want_dts;
  };

  testcase tests[] = {
    {
      "testOne",
      1e-10,
      1.0,
      {{1, 1, 1}, {0, 0, 0}, {.5, .5, .5}, {.9, .9, .9}},
      {1, 0, 0.5, 1.0},
      {1, 1, 0.5, 0.5},
    }
  };

  double tol = 1e-10;

  for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    double dt = 0;
    std::vector<std::vector<double> > solns = tests[i].solns;
    std::vector<double> want_dts = tests[i].want_dts;
    std::vector<double> want_times = tests[i].want_times;

    std::map<double, StepperInfo> snaps;
    DT2Stepper s(tol, tests[i].e_tol, tests[i].e_max);
    StepperInfo si = blankInfo();
    si.prev_dt = 1; // initial dt

    libMesh::Parallel::Communicator dummy_comm;
    int n = tests[i].solns[0].size();
    si.soln_nonlin = NumericVector<Number>::build(dummy_comm);
    si.soln_nonlin->init(n, n, false, SERIAL);

    std::stringstream ss;
    ss << "case " << i+1 << " (" << tests[i].title << "):\n";
    for (int j = 0; j < solns.size(); j++)
    {
      StepperFeedback sf = {};
      dt = s.advance(&si, &sf);
      updateInfo(&si, &sf, dt, &snaps);
      *si.soln_nonlin = solns[j];
      ss << "    step " << j+1 << "\n";
      if (std::abs(want_times[j] - si.time) > tol || std::abs(want_dts[j] - si.prev_dt) > tol)
      {
        ss << "        time: want " << want_times[j] << ", got " << si.time << "\n";
        ss << "        dt  : want " << want_dts[j] << ", got " << si.prev_dt << "\n";
        printf(ss.str().c_str());
        CPPUNIT_ASSERT(false);
      }
      else
      {
        ss << "        time: got "  << si.time << "\n";
        ss << "        dt  : got " << si.prev_dt << "\n";
      }
    }
  }
}

void
StepperTest::scratch()
{
  std::string str = "(MinOfStepper (ConstStepper 4.2) (FixedPointStepper (2 4 10 12) 1e-10) 1e-10)";
  std::vector<StepperToken> toks = lexStepper(str);
  for (auto& tok : toks) {
    //std::c out << tok.str() << "\n";
  }
  StepperNode nd = parseStepper(lexStepper(str));
  //std::c out << nd.str();

  Stepper* s = buildStepper(nd);
  if (!s)
    throw Err("got nullptr from buildStepper");
  StepperInfo si = blankInfo();

  for (int j = 0; j < 10; j++)
  {
    double dt = s->advance(&si, nullptr);
    //std::c out << "time=" << si.time << ", dt=" << dt << "\n";
    updateInfo(&si, nullptr, dt);
  }
  return;

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

  //std::c out << vec(0) << "\n";
  //std::c out << vec(1) << "\n";
  //std::c out << vec(2) << "\n";

  //vec += vec2;

  //std::c out << vec(0) << "\n";
  //std::c out << vec(1) << "\n";
  //std::c out << vec(2) << "\n";
}
