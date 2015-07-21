#include "GrainAdvectionVelocity.h"

template<>
InputParameters validParams<GrainAdvectionVelocity>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculation the advection velocity of grain due to rigid vody translation and rotation");
  params.addCoupledVar("etas", "Array of other coupled order parameters");
  params.addParam<Real>("translation_constant",500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant",1.0, "constant value characterizing grain rotation");
  params.addParam<UserObjectName>("grain_data","userobject for getting volume and center of mass of grains");
  params.addParam<UserObjectName>("grain_force","userobject for getting force and torque acting on grains");
  return params;
}

GrainAdvectionVelocity::GrainAdvectionVelocity(const std::string & name, InputParameters parameters) :
   Material(name,parameters),
   _grain_data(getUserObject<ComputeGrainCenterUserObject>("grain_data")),
   _grain_volumes(_grain_data.getGrainVolumes()),
   _grain_centers(_grain_data.getGrainCenters()),
   _grain_force_torque(getUserObject<ComputeGrainForceAndTorque>("grain_force")),
   _grain_forces(_grain_force_torque.getForceValues()),
   _grain_torques(_grain_force_torque.getTorqueValues()),
   _grain_force_derivatives(_grain_force_torque.getForceDerivatives()),
   _grain_torque_derivatives(_grain_force_torque.getTorqueDerivatives()),
   _mt(getParam<Real>("translation_constant")),
   _mr(getParam<Real>("rotation_constant")),
   _ncrys(_grain_forces.size()),
   _vals(_ncrys), //Size variable arrays
   _grad_vals(_ncrys),
   _velocity_advection(declareProperty<std::vector<RealGradient> >("advection_velocity")),
   _div_velocity_advection(declareProperty<std::vector<Real> >("advection_velocity_divergence")),
   _velocity_advection_derivative(declareProperty<std::vector<RealGradient> >("advection_velocity_derivative")),
   _div_velocity_advection_derivative(declareProperty<std::vector<Real> >("advection_velocity_divergence_derivative"))
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
  RealGradient velocity_translation;
  Real div_velocity_translation;
  RealGradient velocity_rotation;
  Real div_velocity_rotation;

  RealGradient velocity_translation_derivative;
  Real div_velocity_translation_derivative;
  RealGradient velocity_rotation_derivative;
  Real div_velocity_rotation_derivative;

  _velocity_advection[_qp].resize(_ncrys);
  _div_velocity_advection[_qp].resize(_ncrys);
  _velocity_advection_derivative[_qp].resize(_ncrys);
  _div_velocity_advection_derivative[_qp].resize(_ncrys);

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    velocity_translation = _mt / _grain_volumes[i] * ((*_vals[i])[_qp] * _grain_forces[i]);
    div_velocity_translation = _mt / _grain_volumes[i] * ((*_grad_vals[i])[_qp] * _grain_forces[i]);
    velocity_rotation = _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    div_velocity_rotation = _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp] ;

    velocity_translation_derivative = _mt / _grain_volumes[i] * ((*_vals[i])[_qp] * _grain_force_derivatives[i]);
    div_velocity_translation_derivative = _mt / _grain_volumes[i] * ((*_grad_vals[i])[_qp] * _grain_force_derivatives[i]);
    velocity_rotation_derivative = _mr / _grain_volumes[i] * (_grain_torque_derivatives[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    div_velocity_rotation_derivative = _mr / _grain_volumes[i] * (_grain_torque_derivatives[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp] ;

    _velocity_advection[_qp][i] = velocity_translation + velocity_rotation;
    _div_velocity_advection[_qp][i] = div_velocity_translation + div_velocity_rotation;
    _velocity_advection_derivative[_qp][i] = velocity_translation_derivative + velocity_rotation_derivative;
    _div_velocity_advection_derivative[_qp][i] = div_velocity_translation_derivative + div_velocity_rotation_derivative;
  }
}
