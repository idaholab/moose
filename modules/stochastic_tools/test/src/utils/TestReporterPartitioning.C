//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestReporterPartitioning.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", TestReporterPartitioning);

InputParameters
TestReporterPartitioning::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<SamplerName>("sampler", "Sampler defining the partitioning.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "reporters", "Vector reporters to compare with sampler partioning.");
  return params;
}

TestReporterPartitioning::TestReporterPartitioning(const InputParameters & parameters)
  : GeneralReporter(parameters), _sampler(getSampler("sampler"))
{
  for (const auto & rname : getParam<std::vector<ReporterName>>("reporters"))
    _reporters[rname] = &getReporterValueByName<std::vector<Real>>(rname);
}

void
TestReporterPartitioning::execute()
{
  std::stringstream ss;
  for (const auto & it : _reporters)
  {
    const ReporterName & rname = it.first;
    const dof_id_type rsize = it.second->size();

    dof_id_type expect_size;
    const auto & mode = _fe_problem.getReporterData().getReporterMode(rname);
    if (mode == REPORTER_MODE_DISTRIBUTED || (mode == REPORTER_MODE_ROOT && processor_id() != 0))
      expect_size = _sampler.getNumberOfLocalRows();
    else if (mode == REPORTER_MODE_REPLICATED ||
             (mode == REPORTER_MODE_ROOT && processor_id() == 0))
      expect_size = _sampler.getNumberOfRows();
    else
      mooseError("Reporter decalred with unsupported mode.");

    if (rsize != expect_size)
      ss << std::endl
         << "   " << rname << ": expected size = " << expect_size << ", actual size = " << rsize;
  }

  if (!ss.str().empty())
    mooseError("The following reporter values are not partitioning the same as ",
               getParam<SamplerName>("sampler"),
               ":",
               ss.str());
}
