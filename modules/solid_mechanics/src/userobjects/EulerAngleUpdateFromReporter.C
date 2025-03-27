//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EulerAngleUpdateFromReporter.h"

#include <fstream>

registerMooseObject("SolidMechanicsApp", EulerAngleUpdateFromReporter);

InputParameters
EulerAngleUpdateFromReporter::validParams()
{
  InputParameters params = EulerAngleFileReader::validParams();
  params.addClassDescription("Update Euler angle from reporter value.");
  params.addRequiredParam<ReporterName>(
      "euler_angle_0_name",
      "reporter name for the first component of the Euler angles in degrees.  This "
      "parameter uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "euler_angle_1_name",
      "reporter name for the second component of the Euler angles in degrees.  This "
      "parameter uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "euler_angle_2_name",
      "reporter name for the third component of the Euler angles in degrees.  This "
      "parameter uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>("grain_id_name",
                                        "reporter name for the grain IDs.  This "
                                        "parameter uses the reporter syntax <reporter>/<name>.");
  return params;
}

EulerAngleUpdateFromReporter::EulerAngleUpdateFromReporter(const InputParameters & params)
  : EulerAngleFileReader(params),
    _euler_angle_0(
        getReporterValue<std::vector<Real>>("euler_angle_0_name", REPORTER_MODE_REPLICATED)),
    _euler_angle_1(
        getReporterValue<std::vector<Real>>("euler_angle_1_name", REPORTER_MODE_REPLICATED)),
    _euler_angle_2(
        getReporterValue<std::vector<Real>>("euler_angle_2_name", REPORTER_MODE_REPLICATED)),
    _grain_id(getReporterValue<std::vector<Real>>("grain_id_name", REPORTER_MODE_REPLICATED)),
    _first_time(true)
{
}

void
EulerAngleUpdateFromReporter::initialize()
{
  // only update Euler angles info from the reporter when the initial Euler angle has been read from
  // file
  if (_first_time)
  {
    _angles.clear();
    EulerAngleFileReader::readFile();
    _first_time = false;
  }
  else
    UpdateEulerAngle();
}

void
EulerAngleUpdateFromReporter::UpdateEulerAngle()
{
  // check sizes of the containers
  if (_grain_id.size() != _euler_angle_0.size() || _grain_id.size() != _euler_angle_1.size() ||
      _grain_id.size() != _euler_angle_2.size())
    paramError("grain_id_name", "Number of reporters' entries do not match.");

  // Note here the size of the `_angles` and `_grain_id` can differ, as we resize the `_angles`
  // container based on the largest grain ID. This can be improved by having `_angles` as a
  // unordered_map which stores grain ID and EulerAngles pairs.

  // zip the grain id with the euler angles
  std::map<int, EulerAngles> ea_data;
  for (const auto i : index_range(_grain_id))
  {
    unsigned int gid = (unsigned int)(_grain_id[i]);
    ea_data[gid] = EulerAngles(_euler_angle_0[i], _euler_angle_1[i], _euler_angle_2[i]);
  }

  // re-assign euler angles based on the new data
  auto max_grain_id = ea_data.rbegin()->first;
  _angles.clear();
  _angles.resize(max_grain_id + 1); // make sure _angles[max_grain_id] is valid

  for (const auto it : ea_data)
  {
    _angles[it.first] = it.second;
  }
}
