//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorOfVectorRowSum.h"

registerMooseObject("OptimizationApp", VectorOfVectorRowSum);

InputParameters
VectorOfVectorRowSum::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Row sum from a vector of vectors into a single vector.");

  params.addParam<std::string>("name",
                               "VectorOfVectorRowSum",
                               "Name of reporter that contains row sum from vector of vectors.");
  params.addRequiredParam<ReporterName>(
      "reporter_vector_of_vectors", "Name of StochasticReporter vector of vectors to be summed");
  // should execute on Timestep end to make sure data has been cloned into reporter from subapps
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

VectorOfVectorRowSum::VectorOfVectorRowSum(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _row_sum(declareValueByName<std::vector<Real>>(getParam<std::string>("name"),
                                                   REPORTER_MODE_REPLICATED)),
    _rname(getParam<ReporterName>("reporter_vector_of_vectors"))
{
}

void
VectorOfVectorRowSum::finalize()
{
  // Reporter is gotten in finalize instead of intial_setup because it needs to work for
  // StochasticReporter which is cloned into by SamplerReporterTransfer
  _reporter_data =
      &getReporterValueByName<std::vector<std::vector<Real>>>(_rname, REPORTER_MODE_REPLICATED);
  _row_sum.clear();
  std::size_t entries(_reporter_data->at(0).size());
  _row_sum.resize(entries, 0.);
  for (auto & reporter_vector : *_reporter_data)
  {
    if (reporter_vector.size() != entries)
      mooseError("Every vector in 'reporter_vector_of_vectors=",
                 _rname,
                 "' must be the same size.",
                 "\nFirst Vector size = ",
                 entries,
                 "\nCurrent Vector size = ",
                 reporter_vector.size());
    for (std::size_t i = 0; i < entries; ++i)
      _row_sum[i] += reporter_vector[i];
  }
}
