
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiApp.h"
#include "MooseError.h"

#include "gtest_include.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <string>

struct Case
{
  unsigned int nprocs;
  unsigned int nsims;
  unsigned int min_app_procs;
  unsigned int max_app_procs;
  unsigned int batch_mode;

  // map[apps/proc --> num procs with that many apps]
  std::map<int, int> apps_per_proc;
  // map[procs/app --> num apps with that many procs]
  std::map<int, int> procs_per_app;
  // map[sims/proc --> num procs with that many sims]
  std::map<int, int> sims_per_proc;
  // map[procs/sim --> num sims with that many procs]
  std::map<int, int> procs_per_sim;
};

std::string
caseStr(const Case & c)
{
  std::stringstream ss;
  ss << c.nprocs << " procs for " << c.nsims << " apps (" << c.min_app_procs << " to "
     << c.max_app_procs << " procs)";
  return ss.str();
}

TEST(MultiAppTests, OldRevamp)
{
  Moose::_throw_on_error = true;

  // clang-format off
  Case cases[] = {
   //                          sum(napps) = total_napps
   //                          sum(nprocs) = total_nprocs
   //                      and sum(apps/proc * nprocs) = procs_per_app * napps (for non-batch mode)
   // nprocs nsims min  max  batch  {apps/proc,nprocs}...        {procs/app, napps}...     {sims/proc, nprocs}... {procs/sim, nsims}...
      {10,     1,  1,   999, false,    {{1, 10}},                 {{10, 1}},                 {},                   {}},
      { 1,    10,  1,   999, false,    {{10, 1}},                 {{1, 10}},                 {},                   {}},
      {10,     1,  1,     3, false,    {{1, 3}, {0, 7}},          {{3, 1}},                  {},                   {}},
      {10,     1,  1,     1, false,    {{1, 1}, {0, 9}},          {{1, 1}},                  {},                   {}},
      {10,    23,  1,   999, false,    {{3, 3}, {2, 7}},          {{1, 23}},                 {},                   {}},
      {23,    10,  1,   999, false,    {{1, 23}},                 {{3, 3}, {2, 7}},          {},                   {}},
      {23,    10,  7,     7, false,    {{3, 14}, {4, 7}, {0, 2}}, {{7, 10}},                 {},                   {}},
      {10,    23,  3,   999, false,    {{8, 7}, {7, 3}},          {{4, 8}, {3, 15}},         {},                   {}},
      {10,    23,  1,   999, true,     {{1, 10}},                 {{1, 10}, {0, 13}},        {{3, 3}, {2, 7}},     {{1, 23}}},
      {10,    23,  3,   999, true,     {{1, 10}},                 {{3, 2}, {4, 1}, {0, 20}}, {{8, 7}, {7, 3}},     {{4, 8}, {3, 15}}},
  };
  // clang-format on

  std::stringstream err;
  for (size_t i = 0; i < sizeof(cases) / sizeof(Case); i++)
  {
    auto test = cases[i];

    std::map<int, int> procs_per_app;
    std::map<int, int> apps_per_proc;
    std::map<int, int> procs_per_sim;
    std::map<int, int> sims_per_proc;
    std::map<int, int> app_to_nprocs;
    std::map<int, int> sim_to_nprocs;
    for (unsigned int rank = 0; rank < test.nprocs; rank++)
    {
      auto config = rankConfig(
          rank, test.nprocs, test.nsims, test.min_app_procs, test.max_app_procs, test.batch_mode);
      apps_per_proc[config.num_local_apps] += 1;
      sims_per_proc[config.num_local_sims] += 1;
      for (unsigned int n = 0; n < config.num_local_apps; n++)
        app_to_nprocs[config.first_local_app_index + n] += 1;
      for (unsigned int n = 0; n < config.num_local_sims; n++)
        sim_to_nprocs[config.first_local_sim_index + n] += 1;
    }

    for (unsigned int n = 0; n < test.nsims; n++)
      procs_per_app[app_to_nprocs[n]] += 1;

    for (auto entry : procs_per_app)
    {
      auto key = entry.first;
      auto napps = entry.second;
      auto want = test.procs_per_app[key];
      auto got = procs_per_app[key];
      if (napps != want)
        err << "case " << i + 1 << " FAIL " << caseStr(test) << ": at " << key << " procs/app want "
            << want << ", got " << got << "\n";
    }
    for (auto entry : apps_per_proc)
    {
      auto key = entry.first;
      auto nprocs = entry.second;
      auto want = test.apps_per_proc[key];
      auto got = apps_per_proc[key];
      if (nprocs != want)
        err << "case " << i + 1 << " FAIL " << caseStr(test) << ": at " << key << " apps/proc want "
            << want << ", got " << got << "\n";
    }

    if (test.batch_mode)
    {
      for (auto entry : procs_per_sim)
      {
        auto key = entry.first;
        auto nsims = entry.second;
        auto want = test.procs_per_sim[key];
        auto got = procs_per_sim[key];
        if (nsims != want)
          err << "case " << i + 1 << " FAIL " << caseStr(test) << ": at " << key
              << " procs/sim want " << want << ", got " << got << "\n";
      }
      for (auto entry : sims_per_proc)
      {
        auto key = entry.first;
        auto nprocs = entry.second;
        auto want = test.sims_per_proc[key];
        auto got = sims_per_proc[key];
        if (nprocs != want)
          err << "case " << i + 1 << " FAIL " << caseStr(test) << ": at " << key
              << " sims/proc want " << want << ", got " << got << "\n";
      }
    }
  }

  auto msg = err.str();
  if (!msg.empty())
    FAIL() << msg;
}
