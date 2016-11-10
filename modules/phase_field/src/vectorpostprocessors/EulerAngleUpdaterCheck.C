/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EulerAngleUpdaterCheck.h"
#include "EulerAngleUpdater.h"
#include "EulerAngleProvider.h"
#include "GrainTrackerInterface.h"
#include "GrainForceAndTorqueInterface.h"
#include "RotationTensor.h"

template<>
InputParameters validParams<EulerAngleUpdaterCheck>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addClassDescription("Provide updated euler angles after rigid body rotation of the grains.");
  params.addRequiredParam<UserObjectName>("grain_tracker_object", "The FeatureFloodCount UserObject to get values from.");
  params.addParam<UserObjectName>("euler_angle_updater", "Name of Euler angle provider user object");
  params.addRequiredParam<UserObjectName>("grain_torques_object", "Name of Euler angle provider user object");
  params.addRequiredParam<VectorPostprocessorName>("grain_volumes", "The feature volume VectorPostprocessorValue.");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  return params;
}

EulerAngleUpdaterCheck::EulerAngleUpdaterCheck(const InputParameters & params) :
    GeneralVectorPostprocessor(params),
    _diff(declareVector("vec_diff")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_object")),
    _euler(getUserObject<EulerAngleUpdater>("euler_angle_updater")),
    _grain_torque(getUserObject<GrainForceAndTorqueInterface>("grain_torques_object")),
    _grain_volumes(getVectorPostprocessorValue("grain_volumes", "feature_volumes")),
    _mr(getParam<Real>("rotation_constant"))
{
}

void
EulerAngleUpdaterCheck::initialize()
{
  const auto grain_num = _grain_tracker.getTotalFeatureCount();
  _angles.resize(grain_num);
  _angles_old.resize(grain_num);
  _diff.assign(3*grain_num, 0.0);

  for (unsigned int i = 0; i < grain_num; ++i)
  {
    _angles[i] = _euler.getEulerAngles(i);
    _angles_old[i] = _euler.getEulerAnglesOld(i);
    RealGradient torque = _grain_torque.getTorqueValues()[i];

    RealVectorValue a(1, 1, 1);
    RotationTensor R(_angles[i]);
    RealVectorValue a_rot = R * a;

    RotationTensor R0(_angles_old[i]);
    RealVectorValue torque_rot = R0 * torque;
    RealVectorValue a_rot0 = R0 * a;

    RealVectorValue torque_rot1;
    RealVectorValue angle_rot;
    torque_rot1(0) = torque_rot(2);
    angle_rot(0) = _mr / _grain_volumes[i] * torque_rot1(0) * _dt;
    torque_rot1(1) = (torque_rot(0) * std::cos(angle_rot(0)) + torque_rot(1) * std::sin(angle_rot(0)));
    angle_rot(1) = _mr / _grain_volumes[i] * torque_rot1(1) * _dt;
    torque_rot1(2) = (torque_rot(0) * std::sin(angle_rot(0)) * std::sin(angle_rot(1))
                 - torque_rot(1) * std::cos(angle_rot(0)) * std::sin(angle_rot(1))
                 + torque_rot(2) * std::cos(angle_rot(1)));
    angle_rot(2) = _mr / _grain_volumes[i] * torque_rot1(2) * _dt;
    angle_rot *= (180.0 / libMesh::pi);

    RotationTensor R4(angle_rot);
    RealVectorValue a_rot1 = R4 * a_rot0;

    _diff[3*i+0] = a_rot(0) - a_rot1(0);
    _diff[3*i+1] = a_rot(1) - a_rot1(1);
    _diff[3*i+2] = a_rot(2) - a_rot1(2);
  }
}
