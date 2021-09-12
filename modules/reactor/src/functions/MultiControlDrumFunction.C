//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiControlDrumFunction.h"

registerMooseObject("ReactorApp", MultiControlDrumFunction);

InputParameters
MultiControlDrumFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "A function that returns an adsorber fraction value for multuple control drums application.");
  params.addRequiredParam<MeshGeneratorName>(
      "mesh_generator",
      "Name of the mesh generator to be used to retreive control drums information.");
  params.addRequiredParam<std::vector<Real>>(
      "angular_speeds", "Vector of angular rotation speeds of the control drum.");
  params.addRequiredParam<std::vector<Real>>(
      "start_angles",
      "Vector of initial anular positions of the beginning of the absorber components.");
  params.addRequiredParam<std::vector<Real>>(
      "angle_ranges", "Vector of angular ranges of the beginning of the absorber components.");
  params.addParam<bool>(
      "use_control_drum_id", true, "Whether extra element id user_control_drum_id is used.");
  params.addParamNamesToGroup("use_control_drum_id", "Advanced");

  return params;
}

MultiControlDrumFunction::MultiControlDrumFunction(const InputParameters & parameters)
  : Function(parameters),
    MeshMetaDataInterface(this),
    ElementIDInterface(this),
    _mesh_generator(getParam<MeshGeneratorName>("mesh_generator")),
    _angular_speeds(getParam<std::vector<Real>>("angular_speeds")),
    _start_angles(getParam<std::vector<Real>>("start_angles")),
    _angle_ranges(getParam<std::vector<Real>>("angle_ranges")),
    _use_control_drum_id(getParam<bool>("use_control_drum_id")),
    _control_drum_id(getElementIDByName("control_drum_id"))
{
  _control_drum_positions =
      getMeshProperty<std::vector<Point>>("control_drum_positions", _mesh_generator);
  _control_drums_azimuthal_meta = getMeshProperty<std::vector<std::vector<Real>>>(
      "control_drums_azimuthal_meta", _mesh_generator);
}

Real
MultiControlDrumFunction::value(Real t, const Point & p) const
{
  if (_angular_speeds.size() != _start_angles.size())
    paramError("start_angles",
               "the size of 'start_angles' must be identical to the size of 'angular_speeds'");
  if (_angular_speeds.size() != _angle_ranges.size())
    paramError("angle_ranges",
               "the size of 'angle_ranges' must be identical to the size of 'angular_speeds'");
  if (_angular_speeds.size() != _control_drum_positions.size())
    paramError("angular_speeds",
               "the size of this parameter must be identical to the control drum number recorded "
               "in the MeshMetaData.");

  // Find the closest control drum for a given Point p; only needed if control drum id is not used.
  std::vector<std::pair<Real, unsigned int>> control_drun_dist_vec;
  if (!_use_control_drum_id)
  {
    for (unsigned int i = 0; i < _control_drum_positions.size(); i++)
    {
      Real cd_dist = std::sqrt(Utility::pow<2>(p(0) - _control_drum_positions[i](0)) +
                               Utility::pow<2>(p(1) - _control_drum_positions[i](1)));
      control_drun_dist_vec.push_back(std::make_pair(cd_dist, i));
    }
    std::sort(control_drun_dist_vec.begin(), control_drun_dist_vec.end());
  }
  const unsigned int cd_id = _use_control_drum_id
                                 ? (_control_drum_id > 0 ? _control_drum_id - 1 : 0)
                                 : control_drun_dist_vec.front().second;
  Point cd_pos = _control_drum_positions[cd_id];

  Real dynamic_start_angle = (_angular_speeds[cd_id] * t + _start_angles[cd_id]) / 180.0 * M_PI;
  Real dynamic_end_angle = dynamic_start_angle + _angle_ranges[cd_id] / 180.0 * M_PI;

  dynamic_start_angle = atan2(std::sin(dynamic_start_angle),
                              std::cos(dynamic_start_angle)) /
                        M_PI * 180.0; // quick way to move to -M_PI to M_PI
  dynamic_end_angle = atan2(std::sin(dynamic_end_angle),
                            std::cos(dynamic_end_angle)) /
                      M_PI * 180.0; // quick way to move to -M_PI to M_PI

  auto start_bound = std::upper_bound(_control_drums_azimuthal_meta[cd_id].begin(),
                                      _control_drums_azimuthal_meta[cd_id].end(),
                                      dynamic_start_angle);
  auto end_bound = std::upper_bound(_control_drums_azimuthal_meta[cd_id].begin(),
                                    _control_drums_azimuthal_meta[cd_id].end(),
                                    dynamic_end_angle);

  // Locate the elements that contain the start/end angles.
  // This part seems long; but it only solve one simple issue -> transition from M_PI to -M_PI
  Real start_low, start_high, end_low, end_high;
  if (start_bound != _control_drums_azimuthal_meta[cd_id].begin() &&
      start_bound != _control_drums_azimuthal_meta[cd_id].end())
  {
    start_low = *(start_bound - 1);
    start_high = *start_bound;
  }
  else
  {
    start_high = _control_drums_azimuthal_meta[cd_id].front();
    start_low = _control_drums_azimuthal_meta[cd_id].back();
  }

  if (end_bound != _control_drums_azimuthal_meta[cd_id].begin() &&
      end_bound != _control_drums_azimuthal_meta[cd_id].end())
  {
    end_low = *(end_bound - 1);
    end_high = *end_bound;
  }
  else
  {
    end_high = _control_drums_azimuthal_meta[cd_id].front();
    end_low = _control_drums_azimuthal_meta[cd_id].back();
  }

  Real azimuthal_p = atan2(p(1) - cd_pos(1), p(0) - cd_pos(0)) / M_PI * 180;

  if (end_high >= start_low)
  {
    if (azimuthal_p >= start_high && azimuthal_p <= end_low)
      return 100.0;
    else if (azimuthal_p <= start_low || azimuthal_p >= end_high)
      return 0.0;
    else if (azimuthal_p < start_high && azimuthal_p > start_low)
    {
      Real start_interval = start_high - start_low;
      Real stab_interval = start_high - dynamic_start_angle;
      return stab_interval / start_interval * 100.0;
    }
    else
    {
      Real end_interval = end_high - end_low;
      Real endab_interval = dynamic_end_angle - end_low;
      return endab_interval / end_interval * 100.0;
    }
  }
  else if (end_low >= end_high)
  {
    if (azimuthal_p >= start_high && azimuthal_p <= end_low)
      return 100.0;
    else if (azimuthal_p <= start_low && azimuthal_p >= end_high)
      return 0.0;
    else if (azimuthal_p < start_high && azimuthal_p > start_low)
    {
      Real start_interval = start_high - start_low;
      Real stab_interval = start_high - dynamic_start_angle;
      return stab_interval / start_interval * 100.0;
    }
    else
    {
      Real end_interval = end_high - end_low + 360.0;
      Real endab_interval = (dynamic_end_angle - end_low >= 0.0)
                                ? (dynamic_end_angle - end_low)
                                : (dynamic_end_angle - end_low + 360.0);
      return endab_interval / end_interval * 100.0;
    }
  }
  else if (start_high >= end_low)
  {
    if (azimuthal_p >= start_high || azimuthal_p <= end_low)
      return 100.0;
    else if (azimuthal_p >= end_high && azimuthal_p <= start_low)
      return 0.0;
    else if (azimuthal_p < start_high && azimuthal_p > start_low)
    {
      Real start_interval = start_high - start_low;
      Real stab_interval = start_high - dynamic_start_angle;
      return stab_interval / start_interval * 100.0;
    }
    else
    {
      Real end_interval = end_high - end_low;
      Real endab_interval = dynamic_end_angle - end_low;
      return endab_interval / end_interval * 100.0;
    }
  }
  else
  {
    if (azimuthal_p >= start_high && azimuthal_p <= end_low)
      return 100.0;
    else if (azimuthal_p >= end_high && azimuthal_p <= start_low)
      return 0.0;
    else if (azimuthal_p > start_low || azimuthal_p < start_high)
    {
      Real start_interval = start_high - start_low + 360.0;
      Real stab_interval = (start_high - dynamic_start_angle >= 0.0)
                               ? (start_high - dynamic_start_angle)
                               : (start_high - dynamic_start_angle + 360.0);
      return stab_interval / start_interval * 100.0;
    }
    else
    {
      Real end_interval = end_high - end_low;
      Real endab_interval = dynamic_end_angle - end_low;
      return endab_interval / end_interval * 100.0;
    }
  }
}
