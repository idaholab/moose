/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainAdvectionVelocity.h"

template<>
InputParameters validParams<GrainAdvectionVelocity>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculation the advection velocity of grain due to rigid vody translation and rotation");
  params.addRequiredCoupledVarWithAutoBuild("etas", "var_name_base", "op_num", "Array of other coupled order parameters");
  params.addCoupledVar("c", "Concentration field");
  params.addParam<Real>("translation_constant", 500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define type of force density under consideration");
  params.addParam<UserObjectName>("grain_data", "UserObject for get center of mass of grains");
  params.addParam<UserObjectName>("grain_force", "userobject for getting force and torque acting on grains");
  params.addParam<VectorPostprocessorName>("grain_volumes", "The feature volume VectorPostprocessorValue.");
  return params;
}

GrainAdvectionVelocity::GrainAdvectionVelocity(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_data")),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_volumes(getVectorPostprocessorValue("grain_volumes", "feature_volumes")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _mt(getParam<Real>("translation_constant")),
    _mr(getParam<Real>("rotation_constant")),
    _op_num(coupledComponents("etas")),
    _vals(_op_num),
    _grad_vals(_op_num),
    _c_name(getVar("c", 0)->name()),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _velocity_advection(declareProperty<std::vector<RealGradient> >(_base_name + "advection_velocity")),
    _div_velocity_advection(declareProperty<std::vector<Real> >(_base_name + "advection_velocity_divergence"))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("etas", i);
    _grad_vals[i] = &coupledGradient("etas",i);
  }
  mooseWarning("GrainAdvectionVelocity is no longer used by RBM kernels, it can be used for visualization only.");
}

void
GrainAdvectionVelocity::computeQpProperties()
{
  auto grain_num = _grain_tracker.getTotalFeatureCount();

  _velocity_advection[_qp].resize(grain_num);
  _div_velocity_advection[_qp].resize(grain_num);

  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (unsigned int i = 0; i < _grain_volumes.size(); ++i)
  {
    mooseAssert(i < _grain_volumes.size(), "grain index is out of bounds");
    const auto volume = _grain_volumes[i];
    const auto centroid = _grain_tracker.getGrainCentroid(i);

    for (unsigned int j = 0; j < _op_num; ++j)
      if (i == op_to_grains[j])
      {
        const RealGradient velocity_translation = _mt / volume * ((*_vals[j])[_qp] * _grain_forces[i]);
        const Real div_velocity_translation = _mt / volume * ((*_grad_vals[j])[_qp] * _grain_forces[i]);
        const RealGradient velocity_rotation = _mr / volume * (_grain_torques[i].cross(_q_point[_qp] - centroid)) * (*_vals[j])[_qp];
        const Real div_velocity_rotation = _mr / volume * (_grain_torques[i].cross(_q_point[_qp] - centroid)) * (*_grad_vals[j])[_qp];

        _velocity_advection[_qp][i] = velocity_translation + velocity_rotation;
        _div_velocity_advection[_qp][i] = div_velocity_translation + div_velocity_rotation;
      }
  }
}
