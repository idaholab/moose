/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "OutputEulerAngles.h"
#include "GrainTracker.h"
#include "EulerAngleProvider.h"

template <>
InputParameters
validParams<OutputEulerAngles>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Output euler angles from user object to an AuxVariable.");
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
