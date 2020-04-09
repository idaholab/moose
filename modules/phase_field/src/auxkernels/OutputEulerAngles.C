//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutputEulerAngles.h"
#include "GrainTracker.h"
#include "EulerAngleProvider.h"

registerMooseObject("PhaseFieldApp", OutputEulerAngles);

InputParameters
OutputEulerAngles::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Output Euler angles from user object to an AuxVariable.");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  MooseEnum euler_angles("phi1 Phi phi2");
  params.addRequiredParam<MooseEnum>("output_euler_angle", euler_angles, "Euler angle to output");
  return params;
}

OutputEulerAngles::OutputEulerAngles(const InputParameters & parameters)
  : AuxKernel(parameters),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _grain_tracker(getUserObject<GrainTracker>("grain_tracker")),
    _output_euler_angle(getParam<MooseEnum>("output_euler_angle"))
{
}

void
OutputEulerAngles::precalculateValue()
{
  // ID of unique grain at current point
  const auto grain_id =
      _grain_tracker.getEntityValue((isNodal() ? _current_node->id() : _current_elem->id()),
                                    FeatureFloodCount::FieldType::UNIQUE_REGION,
                                    0);

  // Recover euler angles for current grain
  RealVectorValue angles;
  if (grain_id >= 0)
    angles = _euler.getEulerAngles(grain_id);

  // Return specific euler angle
  _value = angles(_output_euler_angle);
}

Real
OutputEulerAngles::computeValue()
{
  return _value;
}
