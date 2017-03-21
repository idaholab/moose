/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainAdvectionAux.h"

template <>
InputParameters
validParams<GrainAdvectionAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription(
      "Calculates the advection velocity of grain due to rigid body translation and rotation");
  params.addParam<Real>(
      "translation_constant", 1.0, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  params.addParam<UserObjectName>("grain_tracker_object",
                                  "userobject for getting volume and center of mass of grains");
  params.addParam<VectorPostprocessorName>("grain_volumes",
                                           "The feature volume VectorPostprocessorValue.");
  params.addParam<UserObjectName>("grain_force",
                                  "userobject for getting force and torque acting on grains");
  MooseEnum component("x=0 y=1 z=2");
  params.addParam<MooseEnum>("component", component, "The gradient component to compute");
  return params;
}

GrainAdvectionAux::GrainAdvectionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_object")),
    _grain_volumes(getVectorPostprocessorValue("grain_volumes", "feature_volumes")),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _mt(getParam<Real>("translation_constant")),
    _mr(getParam<Real>("rotation_constant")),
    _component(getParam<MooseEnum>("component"))
{
  if (isNodal())
    mooseError("Advection velocity can be assigned to elemental variables only.");
}

void
GrainAdvectionAux::precalculateValue()
{
  // ID of unique grain at current point
  const auto grain_id = _grain_tracker.getEntityValue(
      _current_elem->id(), FeatureFloodCount::FieldType::UNIQUE_REGION, 0);
  if (grain_id >= 0)
  {
    mooseAssert(grain_id < _grain_volumes.size(), "grain index is out of bounds");
    const auto volume = _grain_volumes[grain_id];
    const auto centroid = _grain_tracker.getGrainCentroid(grain_id);

    const RealGradient velocity_translation = _mt / volume * _grain_forces[grain_id];
    const RealGradient velocity_rotation =
        _mr / volume * (_grain_torques[grain_id].cross(_current_elem->centroid() - centroid));
    _velocity_advection = velocity_translation + velocity_rotation;
  }
  else
    _velocity_advection.zero();
}

Real
GrainAdvectionAux::computeValue()
{
  return _velocity_advection(_component);
}
