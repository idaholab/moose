//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorSum.h"

registerMooseObject("OptimizationApp", VectorSum);

InputParameters
VectorSum::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Sum of a vector held in a reporter.");

  params.addParam<std::string>(
      "name", "VectorSum", "Name of reporter containing scalar result of vector sum.");
  params.addParam<Real>("scale", 1, "Scale sum");
  params.addRequiredParam<ReporterName>("vector", "Reporter vector being summed.");
  // This reporters is for postprocessing optimization results and shold be exectuted at the end of
  // execution
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

VectorSum::VectorSum(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _scale(getParam<Real>("scale")),
    _sum(declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_REPLICATED)),
    _vector(getReporterValueByName<std::vector<Real>>(getParam<ReporterName>("vector"),
                                                      REPORTER_MODE_REPLICATED))
{
}

void
VectorSum::finalize()
{
  _sum = 0;
  for (auto & value : _vector)
    _sum += value;
  _sum *= _scale;
}
