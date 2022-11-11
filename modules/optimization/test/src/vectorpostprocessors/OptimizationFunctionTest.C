//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationFunctionTest.h"

#include "OptimizationFunction.h"

registerMooseObject("OptimizationTestApp", OptimizationFunctionTest);

InputParameters
OptimizationFunctionTest::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Outputs parameter gradients for the inputted optimization functions.");
  params.addRequiredParam<std::vector<FunctionName>>("functions",
                                                     "Optimization functions to test.");
  params.addParam<std::vector<Point>>(
      "points",
      std::vector<Point>(1, Point(0, 0, 0)),
      "Points in domain to test the functions, default is single point at (0,0,0).");
  params.addParam<std::vector<Real>>("times",
                                     std::vector<Real>(1, 0),
                                     "Times to test the functions, default is single time at 0.");
  return params;
}

OptimizationFunctionTest::OptimizationFunctionTest(const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _points(getParam<std::vector<Point>>("points")),
    _times(getParam<std::vector<Real>>("times"))
{
  for (const auto & fnm : getParam<std::vector<FunctionName>>("functions"))
  {
    auto func = dynamic_cast<const OptimizationFunction *>(&getFunctionByName(fnm));
    if (!func)
      paramError("functions", fnm, " is not an OptimizationFunction.");
    auto & vecs = _functions[func];
    for (const auto & t : index_range(_times))
      for (const auto & p : index_range(_points))
      {
        const std::string vnm = fnm + "_t" + std::to_string(t) + "_p" + std::to_string(p);
        vecs.push_back(&declareVector(vnm));
      }
  }
}

void
OptimizationFunctionTest::execute()
{
  for (auto & it : _functions)
  {
    const auto & fun = *it.first;
    auto & vecs = it.second;
    std::size_t ind = 0;
    for (const auto & t : _times)
      for (const auto & p : _points)
        (*vecs[ind++]) = fun.parameterGradient(t, p);
  }
}
