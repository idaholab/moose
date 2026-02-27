//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimesReporter.h"
#include "libmesh/parallel_algebra.h"

InputParameters
TimesReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addParam<bool>(
      "unique_times", true, "Whether duplicate values should be removed from the Times vector");
  params.addParam<Real>(
      "unique_tolerance",
      1e-12,
      "Absolute tolerance for removing duplicated times when getting a set of unique times");
  params.addParam<bool>("auto_sort", true, "Whether Times should be sorted");
  // This parameter should be set by each derived class depending on whether the generation of
  // times is replicated or distributed. We want times to be replicated across all ranks
  params.addRequiredParam<bool>("auto_broadcast",
                                "Wether Times should be broadcasted across all ranks");
  params.addParamNamesToGroup("auto_broadcast auto_sort unique_times unique_tolerance", "Advanced");

  // Unlikely to ever be used as Times do not loop over the mesh and use material properties,
  // let alone stateful
  params.suppressParameter<MaterialPropertyName>("prop_getter_suffix");
  params.suppressParameter<bool>("use_interpolated_state");

  params.addParam<bool>("dynamic_time_sequence",
                        true,
                        "Whether the time sequence is dynamic and thus needs to be updated");

  params.registerBase("Times");
  return params;
}

TimesReporter::TimesReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    // Times will be replicated on every rank
    Times(declareValueByName<std::vector<Real>, ReporterVectorContext<Real>>(
              "times", REPORTER_MODE_REPLICATED),
          _fe_problem.time(),
          getParam<bool>("dynamic_time_sequence")),
    _need_broadcast(getParam<bool>("auto_broadcast")),
    _need_sort(getParam<bool>("auto_sort")),
    _need_unique(getParam<bool>("unique_times")),
    _unique_tol(getParam<Real>("unique_tolerance"))
{
}

void
TimesReporter::finalize()
{
  if (_need_broadcast)
    // The consumer/producer reporter interface can keep track of whether a reduction is needed
    // (for example if a consumer needs replicated data, but the producer is distributed) however,
    // we have currently made the decision that times should ALWAYS be replicated
    _communicator.allgather(_times, /* identical buffer lengths = */ false);

  if (_need_sort)
    std::sort(_times.begin(), _times.end());
  if (_need_unique)
    _times.erase(unique(_times.begin(),
                        _times.end(),
                        [this](Real l, Real r) { return std::abs(l - r) < _unique_tol; }),
                 _times.end());
}
