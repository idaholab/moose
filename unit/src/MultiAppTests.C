
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
  unsigned int napps;
  unsigned int min_app_procs;
  unsigned int max_app_procs;

  // map[apps/proc --> num procs with that many apps]
  std::map<int, int> apps_per_proc;
  // map[procs/app --> num apps with that many procs]
  std::map<int, int> procs_per_app;
};

std::string
caseStr(const Case & c)
{
  std::stringstream ss;
  ss << c.nprocs << " procs for " << c.napps << " apps (" << c.min_app_procs << " to "
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
   //                      and ??? sum(apps/proc * nprocs) = total_nprocs
   // nprocs napps min  max  {apps/proc,nprocs}... {procs/app, napps}...
      {10,     1,  1, 999,     {{1, 10}},          {{10, 1}}},
      { 1,    10,  1, 999,     {{10, 1}},          {{1, 10}}},
      {10,     1,  1,   3,     {{1, 3}, {0, 7}},   {{3, 1}}},
      {10,     1,  1,   1,     {{1, 1}, {0, 9}},   {{1, 1}}},
      {10,    23,  1, 999,     {{3, 3}, {2, 7}},   {{1, 23}}},
      {23,    10,  1, 999,     {{1, 23}},          {{3, 3}, {2, 7}}},
  };
  // clang-format on

  std::stringstream err;
  for (size_t i = 0; i < sizeof(cases) / sizeof(Case); i++)
  {
    auto test = cases[i];

    std::map<int, int> procs_per_app;
    std::map<int, int> apps_per_proc;
    std::map<int, int> app_to_nprocs;
    for (unsigned int rank = 0; rank < test.nprocs; rank++)
    {
      auto config =
          rankConfig(rank, test.nprocs, test.napps, test.min_app_procs, test.max_app_procs);
      apps_per_proc[config.num_local_apps] += 1;
      for (unsigned int n = 0; n < config.num_local_apps; n++)
        app_to_nprocs[config.first_local_app_index + n] += 1;
    }

    for (unsigned int n = 0; n < test.napps; n++)
      procs_per_app[app_to_nprocs[n]] += 1;

    for (auto entry : procs_per_app)
    {
      auto key = entry.first;
      auto napps = entry.second;
      auto want = test.procs_per_app[key];
      if (napps != want)
        err << "case " << i + 1 << " FAIL " << caseStr(test) << ": at " << key << " procs/app want "
            << want << ", got " << napps << "\n";
    }
    for (auto entry : apps_per_proc)
    {
      auto key = entry.first;
      auto nprocs = entry.second;
      auto want = test.apps_per_proc[key];
      if (nprocs != want)
        err << "case " << i + 1 << " FAIL " << caseStr(test) << ": at " << key << " apps/proc want "
            << want << ", got " << nprocs << "\n";
    }
  }

  auto msg = err.str();
  if (!msg.empty())
    FAIL() << msg;
}
