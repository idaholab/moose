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

CPPUNIT_TEST_SUITE_REGISTRATION(StepperTest);

StepperInfo blankInfo();
void updateInfo(StepperInfo * si, StepperFeedback * sf, Real dt,
                std::map<Real, StepperInfo> * snaps = nullptr);
void cloneStepperInfo(StepperInfo * src, StepperInfo * dst);

struct BasicTest
{
  std::string title;
  Real tol;
  // time step on which actual used dt is half (negative) Real (positive) what
  // was returned
  int wrong_dt;
  StepperBlock * stepper;
  std::vector<bool> convergeds;
  std::vector<Real> want_dts;
};

void
StepperTest::baseSteppers()
{
  StepperBlock::logging(false);

  Real tol = 1e-10;
  Real inf = std::numeric_limits<Real>::infinity();

  // clang-format off
  BasicTest tests[] = {
    {
      "fixedTimes zero-len-seq",
      tol,
      0,
      BaseStepper::fixedTimes({}, tol),
      { true },
      { inf }
    }, { "fixedTimes normal-seq",
      1e-10,
      false,
      BaseStepper::fixedTimes({ 1, 2, 5 }, tol),
      { true, true, true, true },
      { 1, 1, 3, inf }
    }, {
      // checks that the stepper doesn't repeat time t0 with a dt=0 if a
      // fixed-point time is equal to the initial time.
      "fixedTimes want-point-t0-on-same",
      tol,
      0,
      BaseStepper::fixedTimes({ 0, 1, 3 }, tol),
      { true, true, true, true },
      { 1, 2, inf, inf }
    }, {
      "fixedTimes under-tolerance",
      tol,
      0,
      BaseStepper::fixedTimes({ 1e-11, 1, 3 }, tol),
      { true, true, true, true },
      { 1, 2, inf, inf }
    }, {
      "fixedTimes over-tolerance",
      tol,
      0,
      BaseStepper::fixedTimes({ 1e-9, 1, 3 }, tol),
      { true, true, true, true },
      { 1e-9, 1 - 1e-9, 2, inf }
    }, {
      "fixedTimes violate-dt-under",
      tol,
      -1,
      BaseStepper::fixedTimes({ 1, 2, 5 }, tol),
      { true, true, true, true },
      { 0.5, 0.5, 1, 3 }
    }, {
      "fixedTimes violate-dt-over",
      tol,
      +1,
      BaseStepper::fixedTimes({ 1, 2, 5 }, tol),
      { true, true, true, true },
      { 2, 3, inf, inf }
    }, {
      "everyN  const",
      tol,
      0,
      BaseStepper::everyN(BaseStepper::fixedTimes({ 1, 3, 5 }, tol), 2),
      { true, true, true, true },
      { 1, 1, 1, 1 }
    }, {
      "maxRatio constrained",
      tol,
      0,
      BaseStepper::maxRatio(BaseStepper::fixedTimes({ 1, 2, 5 }, tol), 2),
      { true, true, true, true },
      { 1, 1, 2, 1 }
    }, {
      "maxRatio unconstrained",
      tol,
      0,
      BaseStepper::maxRatio(BaseStepper::fixedTimes({ 1, 2, 5 }, tol), 3),
      { true, true, true, true },
      { 1, 1, 3, 9 }
    }
  };
  // clang-format on

  tableTestBasic(tests);
}

void
StepperTest::DT2()
{
  StepperBlock::logging(false);

  struct testcase
  {
    std::string title;
    Real e_tol;
    Real e_max;
    std::vector<std::vector<Real>> solns;
    std::vector<Real> want_dts;
    std::vector<bool> convergeds;
    std::vector<Real> want_times;
  };
  // clang-format off

  // Initial prev_dt is set to one - which DT2 will use for its first returned dt.
  // The first solution vector is set *after* the first dt and time step.
  // the times are compared after applying the most recent dt.
  testcase tests[] = {
    {
        "testConvergeAll",
        1e-10,
        1.0,
        { { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 } },
        { 1, 1, 0.5, 0.5 },
        { true, true, true, true },
        { 1, 0, 0.5, 1.0 },
    }, {
        "testConvergeFailBeforeRewind",
        1e-10,
        1.0,
        { { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 } },
        { 1, 1, 1, 0.5, 0.5 },
        { false, true, true, true, true },
        { 0, 1, 0, 0.5, 1.0 },
    }, {
        "testConvergeFailOnRewind",
        1e-10,
        1.0,
        { { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 } },
        { 1, 1, 1, 1, 0.5, 0.5 },
        { true, false, true, true, true, true },
        { 1, 0, 1, 0, 0.5, 1 },
    }, {
        "testConvergeFailAfterRewind",
        1e-10,
        1.0,
        { { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 } },
        { 1, 1, 0.5, 1, 0.5, 0.5, 0.5 },
        { true, true, false, true, true, true, true },
        { 1, 0, 0, 1, 0, 0.5, 1 },
    }, {
        "testConvergeFailBeforeFinal",
        1e-10,
        1.0,
        { { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 } },
        { 1, 1, 0.5, 0.5, 1, 0.5, 0.5, 0.5 },
        { true, true, true, false, true, true, true, true },
        { 1, 0, 0.5, 0.5, 1.5, 0.5, 1.0, 1.5 },
    }, {
        "testConvergeAll-more",
        1e-2,
        1.0,
        { { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .85, .85, .85 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 }, { .9, .9, .9 } },
        { 1, 1, 0.5, 0.5, .18, .5, .09, .09 },
        { true, true, true, true, true, true, true, true },
        { 1, 0, 0.5, 1, 1.18, 1, 1.09, 1.18 },
    }
  };
  // clang-format on

  Real tol = 1e-10;
  Real integrator_order = 1;

  for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    Real dt = 0;
    std::vector<std::vector<Real>> solns = tests[i].solns;
    std::vector<Real> want_dts = tests[i].want_dts;
    std::vector<Real> want_times = tests[i].want_times;

    std::map<Real, StepperInfo> snaps;
    DT2Block s(tol, tests[i].e_tol, tests[i].e_max, integrator_order);
    StepperInfo si = blankInfo();
    si.prev_dt = 1; // initial dt

    libMesh::Parallel::Communicator dummy_comm;
    int n = tests[i].solns[0].size();
    si.soln_nonlin.reset(NumericVector<Number>::build(dummy_comm).release());
    si.soln_nonlin->init(n, n, false, SERIAL);
    si.converged = true;
    std::stringstream ss;
    ss << "case " << i + 1 << " (" << tests[i].title << "):\n";
    for (int j = 0; j < solns.size(); j++)
    {
      StepperFeedback sf = {};
      dt = s.next(si, sf);
      si.converged = tests[i].convergeds[j];
      updateInfo(&si, &sf, dt, &snaps);
      *si.soln_nonlin = solns[j];
      ss << "    step " << j + 1 << "\n";
      if (std::abs(want_times[j] - si.time) > tol ||
          std::abs(want_dts[j] - si.prev_dt) > tol)
      {
        ss << "        time: want " << want_times[j] << ", got " << si.time
           << "\n";
        ss << "        dt  : want " << want_dts[j] << ", got " << si.prev_dt
           << "\n";
        printf(ss.str().c_str());
        CPPUNIT_ASSERT(false);
      }
      else
      {
        ss << "        time: got " << si.time << "\n";
        ss << "        dt  : got " << si.prev_dt << "\n";
      }
    }
    printf(ss.str().c_str());
  }
}

void
StepperTest::scratch()
{
  std::string str = "(MinOfStepper (ConstStepper 4.2) (FixedPointStepper (2 4 "
                    "10 12) 1e-10) 1e-10)";
  std::vector<StepperToken> toks = lexStepper(str);
  for (auto & tok : toks)
  {
    // std::c out << tok.str() << "\n";
  }
  StepperNode nd = parseStepper(lexStepper(str));
  // std::c out << nd.str();

  StepperBlock::Ptr s(buildStepper(nd));
  if (!s)
    throw Err("got nullptr from buildStepper");
  StepperInfo si = blankInfo();
  StepperFeedback sf = {};

  for (int j = 0; j < 10; j++)
  {
    Real dt = s->next(si, sf);
    // std::c out << "time=" << si.time << ", dt=" << dt << "\n";
    updateInfo(&si, nullptr, dt);
  }
  return;
}

void
cloneStepperInfo(StepperInfo * src, StepperInfo * dst)
{
  dst->step_count = src->step_count;
  dst->time = src->time;
  dst->prev_dt = src->prev_dt;
  dst->prev_prev_dt = src->prev_prev_dt;
  dst->prev_prev_prev_dt = src->prev_prev_prev_dt;
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
updateInfo(StepperInfo * si, StepperFeedback * sf, Real dt,
           std::map<Real, StepperInfo> * snaps)
{
  if (sf && sf->rewind)
  {
    cloneStepperInfo(&(*snaps)[sf->rewind_time], si);
    return;
  }
  else if (sf && snaps && sf->snapshot)
  {
    cloneStepperInfo(si, &(*snaps)[si->time]);
  }
  if (si->converged)
    si->time += dt;
  si->prev_prev_prev_dt = si->prev_prev_dt;
  si->prev_prev_dt = si->prev_dt;
  si->prev_dt = dt;
  si->step_count++;
}

StepperInfo
blankInfo()
{
  return {1, 0, 0, 0, 0, 0, 0, true, true, 0, 0, 0, nullptr, nullptr, nullptr};
}

void
StepperTest::tableTestBasic(BasicTest tests[])
{
  for (int i = 0; i < sizeof(tests); i++)
  {
    BasicTest test = tests[i];
    Real dt = 0;
    std::vector<Real> want_dts = test.want_dts;
    StepperBlock * s = test.stepper;
    Real tol = test.tol;
    int wrong_dt = test.wrong_dt;

    StepperInfo si = blankInfo();
    si.converged = true;
    std::stringstream ss;
    ss << "case " << i + 1 << " (" << test.title << "):\n";
    for (int j = 0; j < want_dts.size(); j++)
    {
      StepperFeedback sf = {};
      dt = s->next(si, sf);
      if (j == std::abs(wrong_dt) - 1 && wrong_dt > 0)
        dt *= 2;
      else if (j == std::abs(wrong_dt) - 1 && wrong_dt < 0)
        dt /= 2;
      si.converged = test.convergeds[j];
      updateInfo(&si, &sf, dt, nullptr);
      ss << "    step " << j + 1 << "\n";
      if (std::abs(want_dts[j] - si.prev_dt) > tol)
      {
        ss << "        dt  : want " << want_dts[j] << ", got " << si.prev_dt
           << "\n";
        printf(ss.str().c_str());
        CPPUNIT_ASSERT(false);
      }
      else
      {
        ss << "        dt  : got " << si.prev_dt << "\n";
      }
    }
    printf(ss.str().c_str());
    delete s;
  }
}
