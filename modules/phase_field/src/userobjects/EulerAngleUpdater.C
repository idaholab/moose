//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EulerAngleUpdater.h"
#include "GrainTrackerInterface.h"
#include "GrainForceAndTorqueInterface.h"
#include "RotationTensor.h"

registerMooseObject("PhaseFieldApp", EulerAngleUpdater);

InputParameters
EulerAngleUpdater::validParams()
{
  InputParameters params = EulerAngleProvider::validParams();
  params.addClassDescription(
      "Provide updated euler angles after rigid body rotation of the grains.");
  params.addRequiredParam<UserObjectName>("grain_tracker_object",
                                          "The FeatureFloodCount UserObject to get values from.");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");
  params.addRequiredParam<UserObjectName>("grain_torques_object",
                                          "Name of Euler angle provider user object");
  params.addRequiredParam<VectorPostprocessorName>("grain_volumes",
                                                   "The feature volume VectorPostprocessorValue.");
  params.addParam<Real>("rotation_constant", 1.0, "Constant value characterizing grain rotation");
  return params;
}

EulerAngleUpdater::EulerAngleUpdater(const InputParameters & params)
  : EulerAngleProvider(params),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_object")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _grain_torque(getUserObject<GrainForceAndTorqueInterface>("grain_torques_object")),
    _grain_volumes(getVectorPostprocessorValue("grain_volumes", "feature_volumes")),
    _mr(getParam<Real>("rotation_constant")),
    _first_time(true)
{
}

void
EulerAngleUpdater::initialize()
{
  const auto grain_num = _grain_tracker.getTotalFeatureCount();

  if (_first_time)
  {
    _angles.resize(grain_num);
    _angles_old.resize(grain_num);
    for (unsigned int i = 0; i < grain_num; ++i)
      _angles[i] = _euler.getEulerAngles(i); // Read initial euler angles
  }

  unsigned int angle_size = _angles.size();
  for (unsigned int i = angle_size; i < grain_num; ++i) // if new grains are created
    _angles.push_back(_euler.getEulerAngles(i));        // Assign initial euler angles

  for (unsigned int i = 0; i < grain_num; ++i)
  {
    if (!_first_time && !_fe_problem.converged())
      _angles[i] = _angles_old[i];

    RealGradient torque = _grain_torque.getTorqueValues()[i];

    if (i <= angle_size) // if new grains are created
      _angles_old[i] = _angles[i];
    else
      _angles_old.push_back(_angles[i]);

    RotationTensor R0(_angles_old[i]); // RotationTensor as per old euler angles
    RealVectorValue torque_rotated =
        R0 * torque; // Applied torque is rotated to allign with old grain axes
    RealVectorValue omega =
        _mr / _grain_volumes[i] * torque_rotated; // Angular velocity as per old grain axes
    /**
     * Change in euler angles are obtained from the torque & angular velocities about the material
     * axes.
     * Change in phi1, Phi and phi2 are caused by rotation about z axis, x' axis & z'' axis,
     * respectively.
     * Components of the angular velocities across z, x' and z'' axes are obtained from the torque
     * values.
     * This yields change in euler angles due to grain rotation.
     */
    RealVectorValue angle_change;
    angle_change(0) = omega(2) * _dt;
    angle_change(1) =
        (omega(0) * std::cos(angle_change(0)) + omega(1) * std::sin(angle_change(0))) * _dt;
    angle_change(2) = (omega(0) * std::sin(angle_change(0)) * std::sin(angle_change(1)) -
                       omega(1) * std::cos(angle_change(0)) * std::sin(angle_change(1)) +
                       omega(2) * std::cos(angle_change(1))) *
                      _dt;
    angle_change *= (180.0 / libMesh::pi);

    RotationTensor R1(angle_change); // Rotation matrix due to torque
    /**
     * Final RotationMatrix = RotationMatrix due to applied torque X old RotationMatrix
     * Updated Euler angles are obtained by back-tracking the angles from the rotation matrix
     * For details about the componenets of the rotation matrix please refer to RotationTensor.C
     * Phi = acos(R33); phi1 = atan2(R31,-R32); phi2 = atan2(R13,R23) for phi != 0.0 por 180.0
     */
    RealTensorValue R = R1 * R0;

    if (R(2, 2) != 1.0 && R(2, 2) != -1.0) // checks if cos(Phi) = 1 or -1
    {
      _angles[i].phi1 = std::atan2(R(2, 0), -R(2, 1)) * (180.0 / libMesh::pi);
      _angles[i].Phi = std::acos(R(2, 2)) * (180.0 / libMesh::pi);
      _angles[i].phi2 = std::atan2(R(0, 2), R(1, 2)) * (180.0 / libMesh::pi);
    }
    else if (R(2, 2) == 1.0) // special case for Phi = 0.0
    {
      if (R0(2, 2) == 1.0)
        // when Phi_old = 0.0; all the rotations are about z axis and angles accumulates after each
        // rotation
        _angles[i].phi1 = _angles_old[i].phi1 + _angles_old[i].phi2 + angle_change(0);
      else
        _angles[i].phi1 = angle_change(0);
      // Comply with bunge euler angle definitions, 0.0 <= phi1 <= 360.0
      if (std::abs(_angles[i].phi1) > 360.0)
      {
        int laps = _angles[i].phi1 / 360.0;
        _angles[i].phi1 -= laps * 360.0;
      }
      _angles[i].Phi = 0.0;
      _angles[i].phi2 = -_angles[i].phi1 + std::atan2(R(0, 1), R(1, 1)) * (180.0 / libMesh::pi);
    }
    else
    {
      if (R0(2, 2) == 1.0)
        _angles[i].phi1 = _angles_old[i].phi1 + _angles_old[i].phi2 + angle_change(0);
      else
        _angles[i].phi1 = angle_change(0);
      // Comply with bunge euler angle definitions, 0.0 <= phi1 <= 360.0
      if (std::abs(_angles[i].phi1) > 360.0)
      {
        int laps = _angles[i].phi1 / 360.0;
        _angles[i].phi1 -= laps * 360.0;
      }
      _angles[i].Phi = 180.0;
      _angles[i].phi2 = _angles[i].phi1 - std::atan2(-R(0, 1), -R(1, 1)) * (180.0 / libMesh::pi);
    }

    // Following checks and updates are done only to comply with bunge euler angle definitions, 0.0
    // <= phi1/phi2 <= 360.0
    if (_angles[i].phi1 < 0.0)
      _angles[i].phi1 += 360.0;
    if (_angles[i].phi2 < 0.0)
      _angles[i].phi2 += 360.0;
    if (_angles[i].Phi < 0.0)
      mooseError("Euler angle out of range.");
  }

  _first_time = false;
}

unsigned int
EulerAngleUpdater::getGrainNum() const
{
  return _angles.size();
}

const EulerAngles &
EulerAngleUpdater::getEulerAngles(unsigned int i) const
{
  mooseAssert(i < getGrainNum(), "Requesting Euler angles for an invalid grain id");
  return _angles[i];
}

const EulerAngles &
EulerAngleUpdater::getEulerAnglesOld(unsigned int i) const
{
  mooseAssert(i < getGrainNum(), "Requesting Euler angles for an invalid grain id");
  return _angles_old[i];
}
