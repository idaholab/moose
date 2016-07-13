/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainAdvectionVelocity.h"
#include "GrainTrackerInterface.h"

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
  params.addParam<UserObjectName>("grain_data", "userobject for getting volume and center of mass of grains");
  params.addParam<UserObjectName>("grain_force", "userobject for getting force and torque acting on grains");
  return params;
}

GrainAdvectionVelocity::GrainAdvectionVelocity(const InputParameters & parameters) :
   DerivativeMaterialInterface<Material>(parameters),
   _grain_tracker(getUserObject<GrainTrackerInterface>("grain_data")),
   _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
   _grain_forces(_grain_force_torque.getForceValues()),
   _grain_torques(_grain_force_torque.getTorqueValues()),
   _grain_force_derivatives(_grain_force_torque.getForceDerivatives()),
   _grain_torque_derivatives(_grain_force_torque.getTorqueDerivatives()),
   _mt(getParam<Real>("translation_constant")),
   _mr(getParam<Real>("rotation_constant")),
   _ncrys(_grain_forces.size()),
   _vals(_ncrys),
   _grad_vals(_ncrys),
   _c_name(getVar("c", 0)->name()),
   _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
   _velocity_advection(declareProperty<std::vector<RealGradient> >(_base_name + "advection_velocity")),
   _div_velocity_advection(declareProperty<std::vector<Real> >(_base_name + "advection_velocity_divergence")),
   _velocity_advection_derivative_c(declarePropertyDerivative<std::vector<RealGradient> >(_base_name + "advection_velocity", _c_name )),
   _div_velocity_advection_derivative_c(declarePropertyDerivative<std::vector<Real> >(_base_name + "advection_velocity_divergence", _c_name)),
   _velocity_advection_derivative_eta(declarePropertyDerivative<std::vector<RealGradient> >(_base_name + "advection_velocity", "eta"))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _vals[i] = &coupledValue("etas", i);
    _grad_vals[i] = &coupledGradient("etas",i);
  }
}

void
GrainAdvectionVelocity::computeQpProperties()
{
  _velocity_advection[_qp].resize(_ncrys);
  _div_velocity_advection[_qp].resize(_ncrys);
  _velocity_advection_derivative_c[_qp].resize(_ncrys);
  _div_velocity_advection_derivative_c[_qp].resize(_ncrys);
  _velocity_advection_derivative_eta[_qp].resize(_ncrys);

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    const auto volume = _grain_tracker.getGrainVolume(i);
    const auto centroid = _grain_tracker.getGrainCentroid(i);

    const RealGradient velocity_translation = _mt / volume * ((*_vals[i])[_qp] * _grain_forces[i]);
    const Real div_velocity_translation = _mt / volume * ((*_grad_vals[i])[_qp] * _grain_forces[i]);
    const RealGradient velocity_rotation = _mr / volume * (_grain_torques[i].cross(_q_point[_qp] - centroid)) * (*_vals[i])[_qp];
    const Real div_velocity_rotation = _mr / volume * (_grain_torques[i].cross(_q_point[_qp] - centroid)) * (*_grad_vals[i])[_qp];

    const RealGradient velocity_translation_derivative_c = _mt / volume * ((*_vals[i])[_qp] * _grain_force_derivatives[i]);
    const Real div_velocity_translation_derivative_c = _mt / volume * ((*_grad_vals[i])[_qp] * _grain_force_derivatives[i]);
    const RealGradient velocity_rotation_derivative_c = _mr / volume * (_grain_torque_derivatives[i].cross(_q_point[_qp] - centroid)) * (*_vals[i])[_qp];
    const Real div_velocity_rotation_derivative_c = _mr / volume * (_grain_torque_derivatives[i].cross(_q_point[_qp] - centroid)) * (*_grad_vals[i])[_qp] ;

    const RealGradient velocity_translation_derivative_eta = _mt / volume * _grain_forces[i];
    const RealGradient velocity_rotation_derivative_eta = _mr / volume * (_grain_torques[i].cross(_q_point[_qp] - centroid));

    _velocity_advection[_qp][i] = velocity_translation + velocity_rotation;
    _div_velocity_advection[_qp][i] = div_velocity_translation + div_velocity_rotation;
    _velocity_advection_derivative_c[_qp][i] = velocity_translation_derivative_c + velocity_rotation_derivative_c;
    _div_velocity_advection_derivative_c[_qp][i] = div_velocity_translation_derivative_c + div_velocity_rotation_derivative_c;
    _velocity_advection_derivative_eta[_qp][i] = velocity_translation_derivative_eta + velocity_rotation_derivative_eta;
  }
}
