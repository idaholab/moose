//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradientReporterCollector.h"
#include "MooseError.h"
#include "ReporterMode.h"
#include <cstddef>

registerMooseObject("OptimizationApp", GradientReporterCollector);

InputParameters
GradientReporterCollector::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter which collects single parameter contraint gradient into "
                             "global constraint gradients.");
  params.addRequiredParam<ReporterName>("parameter_reporter_name",
                                        "name of the reporter that holds all the parameters.");
  params.addRequiredParam<std::vector<std::vector<ReporterName>>>(
      "reporters", "The gradient reporters to accumulate.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

GradientReporterCollector::GradientReporterCollector(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _parameters(
        getReporterValue<std::vector<Real>>("parameter_reporter_name", REPORTER_MODE_REPLICATED))
{
}

void
GradientReporterCollector::initialSetup()
{
  const ReporterData & rdata = _fe_problem.getReporterData();
  _gradient_vecs.reserve(getParam<std::vector<std::vector<ReporterName>>>("reporters").size());
  size_t params_ind(0);

  for (const auto & rname_vec : getParam<std::vector<std::vector<ReporterName>>>("reporters"))
  {
    for (const auto & rname : rname_vec)
    {
      if (!rdata.hasReporterValue(rname))
        paramError("reporters", "Reporter ", rname, " does not exist.");
      _gradient_vecs[params_ind].push_back(
          &getReporterValue<std::vector<Real>>(rname, REPORTER_MODE_REPLICATED));
      _total_vecs.push_back(&declareValueByName<std::vector<Real>>(rname.getValueName() + "_total",
                                                                   REPORTER_MODE_REPLICATED));
    }
    params_ind++;
  }
}

void
GradientReporterCollector::execute()
{
  const size_t num_params = _parameters.size();
  size_t num_before(0);
  size_t total_vec_ind(0);
  for (const auto & vecs : _gradient_vecs)
  {
    if (vecs.empty())
    {
      // skip empty vecs group
      continue;
    }

    // get the first vector size
    size_t vec_size = vecs[0]->size();

    // Check that all vectors in this group have the same size and if not error out.
    if (!std::all_of(vecs.begin(),
                     vecs.end(),
                     [vec_size](const auto & vec) { return vec->size() == vec_size; }))
    {
      mooseError("Constraint gradient vectors are not all the same size.");
    }

    for (const auto & vec : vecs)
    {
      // clear vector to prepare for setting up
      _total_vecs[total_vec_ind]->clear();
      //   add zeros to locations that have no influence from constraint
      _total_vecs[total_vec_ind]->resize(num_before);
      //   add nonzero gradient values
      _total_vecs[total_vec_ind]->insert(
          _total_vecs[total_vec_ind]->end(), vec->begin(), vec->end());
      //   add zeros to locations that have no influence from constraint
      _total_vecs[total_vec_ind]->resize(num_params);
      total_vec_ind++;
    }
    // On to a new parameter
    num_before += vec_size;
  }
}
