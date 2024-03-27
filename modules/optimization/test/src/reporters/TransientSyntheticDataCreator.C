//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransientSyntheticDataCreator.h"

registerMooseObject("OptimizationTestApp", TransientSyntheticDataCreator);

InputParameters
TransientSyntheticDataCreator::validParams()
{
  InputParameters params = OptimizationDataTempl<GeneralReporter>::validParams();

  params.addClassDescription(
      "Fills optimization data from a forward transient simulation using "
      "a single set of measurement times and points and replicates the data as needed.");
  params.addParam<std::vector<Real>>("measurement_times_for_all_points",
                                     "Measurement time that all points read in a value.");
  params.addParam<std::vector<Point>>("single_set_of_measurement_points",
                                      "One set of measurement points that will be sampled at every "
                                      "time in measurement_times_for_all_points.");
  params.set<std::vector<Real>>("measurement_values") = {0};
  return params;
}

TransientSyntheticDataCreator::TransientSyntheticDataCreator(const InputParameters & parameters)
  : OptimizationDataTempl<GeneralReporter>(parameters)
{
  std::vector<Real> measurement_times_for_all_points;
  if (isParamValid("measurement_times_for_all_points"))
    measurement_times_for_all_points =
        getParam<std::vector<Real>>("measurement_times_for_all_points");
  else
    paramError("measurement_times_for_all_points",
               "Input file must contain measurement_times_for_all_points");

  std::vector<Point> single_set_of_measurement_points;
  if (isParamValid("single_set_of_measurement_points"))
    single_set_of_measurement_points =
        getParam<std::vector<Point>>("single_set_of_measurement_points");
  else
    paramError("single_set_of_measurement_points",
               "Input file must contain single_set_of_measurement_points");

  for (auto & p : single_set_of_measurement_points)
  {
    for (auto & t : measurement_times_for_all_points)
    {
      _measurement_xcoord.push_back(p(0));
      _measurement_ycoord.push_back(p(1));
      _measurement_zcoord.push_back(p(2));
      _measurement_time.push_back(t);
      _measurement_values.push_back(0.0);
    }
  }
}
