//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Distribution.h"
#include "MooseRandom.h"

defineLegacyParams(Distribution);

InputParameters
Distribution::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addRequiredParam<std::string>("type", "class/type name identifying the distribution");
  params.registerBase("Distribution");
  return params;
}

Distribution::Distribution(const InputParameters & parameters)
  : MooseObject(parameters),
    PerfGraphInterface(this),
    _perf_pdf(registerTimedSection("pdf", 4)),
    _perf_cdf(registerTimedSection("cdf", 4)),
    _perf_quantile(registerTimedSection("quantile", 4)),
    _perf_median(registerTimedSection("median", 4))
{
}

Real
Distribution::median() const
{
  mooseError("The distribution '", name(), "' must override the median method.");
  return 0;
}
