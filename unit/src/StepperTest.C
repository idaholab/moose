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

void updateInfo(StepperInfo * si, Real dt, bool next_converged, std::map<Real, StepperInfo> * snaps = nullptr);

struct BasicTest
{
  std::string title;
  Real tol;
  // time step on which actual used dt is half (negative) or twice (positive) what
  // was returned
  int wrong_dt;
  StepperBlock * stepper;
  std::vector<bool> convergeds;
  std::vector<Real> want_dts;
};

void
StepperTest::baseSteppers()
{
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

  tableTestBasic(tests, sizeof(tests) / sizeof(*tests));
}

void
StepperTest::DT2()
{
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
    StepperInfo si;
    si.update(1, 0, 1, 0, 0, true, 0, solns[0], std::vector<Real>(), std::vector<Real>()); // initial dt

    std::stringstream ss;
    ss << "case " << i + 1 << " (" << tests[i].title << "):\n";
    for (int j = 0; j < solns.size(); j++)
    {
      dt = s.next(si);
      updateInfo(&si, dt, tests[i].convergeds[j], &snaps);
      *si.solnNonlin() = solns[j];
      ss << "    step " << j + 1 << "\n";
      if (std::abs(want_times[j] - si.time()) > tol ||
          std::abs(want_dts[j] - si.dt()) > tol)
      {
        ss << "        time: want " << want_times[j] << ", got " << si.time()
           << "\n";
        ss << "        dt  : want " << want_dts[j] << ", got " << si.dt()
           << "\n";
        printf(ss.str().c_str());
        CPPUNIT_ASSERT(false);
      }
      else
      {
        ss << "        time: got " << si.time() << "\n";
        ss << "        dt  : got " << si.dt() << "\n";
      }
    }
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
  StepperInfo si;

  for (int j = 0; j < 10; j++)
  {
    Real dt = s->next(si);
    // std::c out << "time=" << si.time() << ", dt=" << dt << "\n";
    updateInfo(&si, dt, true);
  }
  return;
}

void
updateInfo(StepperInfo * si, Real dt, bool next_converged, std::map<Real, StepperInfo> * snaps)
{
  std::vector<Real> vec;
  vec.resize(si->solnNonlin()->size());
  si->solnNonlin()->localize(vec);

  if (si->rewindTime() != -1)
  {
    *si = (*snaps)[si->rewindTime()];
    si->update(si->stepCount(), si->time(), si->dt(), 0, 0, next_converged, 0, vec, vec, vec);
    return;
  }
  else if (snaps && si->wantSnapshot())
    (*snaps)[si->time()] = *si;

  Real new_t = si->time();
  if (next_converged)
    new_t += dt;
  si->update(si->stepCount()+1, new_t, dt, 0, 0, next_converged, 0, vec, vec, vec);
}

void
StepperTest::tableTestBasic(BasicTest tests[], int n)
{
  for (int i = 0; i < n; i++)
  {
    BasicTest test = tests[i];
    Real dt = 0;
    std::vector<Real> want_dts = test.want_dts;
    StepperBlock * s = test.stepper;
    Real tol = test.tol;
    int wrong_dt = test.wrong_dt;

    StepperInfo si;
    std::stringstream ss;
    ss << "case " << i + 1 << " (" << test.title << "):\n";
    for (int j = 0; j < want_dts.size(); j++)
    {
      dt = s->next(si);
      if (j == std::abs(wrong_dt) - 1 && wrong_dt > 0)
        dt *= 2;
      else if (j == std::abs(wrong_dt) - 1 && wrong_dt < 0)
        dt /= 2;
      updateInfo(&si, dt, test.convergeds[j], nullptr);
      ss << "    step " << j + 1 << "\n";
      if (std::abs(want_dts[j] - si.dt()) > tol)
      {
        ss << "        dt  : want " << want_dts[j] << ", got " << si.dt()
           << "\n";
        printf(ss.str().c_str());
        CPPUNIT_ASSERT(false);
      }
      else
      {
        ss << "        dt  : got " << si.dt() << "\n";
      }
    }
    delete s;
  }
}
