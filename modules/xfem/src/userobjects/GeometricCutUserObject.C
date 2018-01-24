/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GeometricCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

template <>
InputParameters
validParams<GeometricCutUserObject>()
{
  // Get input parameters from parent class
  InputParameters params = validParams<CrackFrontPointsProvider>();

  // Add optional parameters
  params.addParam<Real>("time_start_cut", 0.0, "Start time of geometric cut propagation");
  params.addParam<Real>("time_end_cut", 0.0, "End time of geometric cut propagation");
  // Class description
  params.addClassDescription("Base UserObject class for XFEM Geometric Cuts");
  // Return the parameters
  return params;
}

GeometricCutUserObject::GeometricCutUserObject(const InputParameters & parameters)
    //: GeneralUserObject(parameters), _start_times(), _end_times()
    : CrackFrontPointsProvider(parameters),
      _cut_time_ranges()
{
  _cut_time_ranges.push_back(
      std::make_pair(getParam<Real>("time_start_cut"), getParam<Real>("time_end_cut")));
}

Real
GeometricCutUserObject::cutFraction(unsigned int cut_num, Real time) const
{
  Real fraction = 0.0;

  if (time >= _cut_time_ranges[cut_num].first)
  {
    if (time >= _cut_time_ranges[cut_num].second)
      fraction = 1.0;
    else
      fraction = (time - _cut_time_ranges[cut_num].first) /
                 (_cut_time_ranges[cut_num].second - _cut_time_ranges[cut_num].first);
  }
  return fraction;
}
