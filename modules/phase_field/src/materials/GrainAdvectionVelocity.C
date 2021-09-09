//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainAdvectionVelocity.h"

registerMooseObject("PhaseFieldApp", GrainAdvectionVelocity);

InputParameters
GrainAdvectionVelocity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Calculation the advection velocity of grain due to rigid body translation and rotation");
  params.addRequiredCoupledVarWithAutoBuild(
      "etas", "var_name_base", "op_num", "Array of other coupled order parameters");
  params.addCoupledVar("c", "Concentration field");
  params.addParam<Real>(
      "translation_constant", 500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "type of force density under consideration");
  params.addParam<UserObjectName>("grain_data",
                                  "UserObject for getting the center of mass of grains");
  params.addParam<UserObjectName>("grain_force",
                                  "userobject for getting force and torque acting on grains");
  params.addParam<VectorPostprocessorName>("grain_volumes",
                                           "The feature volume VectorPostprocessorValue.");
  return params;
}

GrainAdvectionVelocity::GrainAdvectionVelocity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_data")),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_volumes(getVectorPostprocessorValue("grain_volumes", "feature_volumes")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _mt(getParam<Real>("translation_constant")),
    _mr(getParam<Real>("rotation_constant")),
    _op_num(coupledComponents("etas")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _velocity_advection(
        declareProperty<std::vector<RealGradient>>(_base_name + "advection_velocity"))
{
  mooseDeprecated("Use GrainAdvectionAux for visualizing advection velocities.");
}

void
GrainAdvectionVelocity::computeQpProperties()
{
  auto grain_num = _grain_tracker.getTotalFeatureCount();
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  _velocity_advection[_qp].resize(grain_num);

  for (unsigned int i = 0; i < _grain_volumes.size(); ++i)
  {
    mooseAssert(i < _grain_volumes.size(), "grain index is out of bounds");
    const auto volume = _grain_volumes[i];
    const auto centroid = _grain_tracker.getGrainCentroid(i);

    for (unsigned int j = 0; j < _op_num; ++j)
      if (i == op_to_grains[j])
      {
        const RealGradient velocity_translation = _mt / volume * _grain_forces[i];
        const RealGradient velocity_rotation =
            _mr / volume * (_grain_torques[i].cross(_current_elem->vertex_average() - centroid));

        _velocity_advection[_qp][i] = velocity_translation + velocity_rotation;
      }
  }
}
