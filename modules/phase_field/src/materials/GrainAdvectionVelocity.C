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
  params.addParam<UserObjectName>("grain_data", "userobject for getting volume and center of mass of grains");
  params.addParam<UserObjectName>("grain_force", "userobject for getting force and torque acting on grains");
  return params;
}

GrainAdvectionVelocity::GrainAdvectionVelocity(const InputParameters & parameters) :
   DerivativeMaterialInterface<Material>(parameters),
   _grain_data(getUserObject<ComputeGrainCenterUserObject>("grain_data")),
   _grain_volumes(_grain_data.getGrainVolumes()),
   _grain_centers(_grain_data.getGrainCenters()),
   _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
   _grain_forces(_grain_force_torque.getForceValues()),
   _grain_torques(_grain_force_torque.getTorqueValues()),
   _grain_force_derivatives(_grain_force_torque.getForceCJacobians()),
  //  _grain_torque_derivatives(_grain_force_torque.getTorqueCJacobians()),
   _mt(getParam<Real>("translation_constant")),
   _mr(getParam<Real>("rotation_constant")),
   _op_num(_grain_forces.size()),
   _vals(_op_num),
   _grad_vals(_op_num),
   _c_name(getVar("c", 0)->name()),
   _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
   _velocity_advection(declareProperty<std::vector<RealGradient> >(_base_name + "advection_velocity")),
   _div_velocity_advection(declareProperty<std::vector<Real> >(_base_name + "advection_velocity_divergence")),
   _velocity_advection_derivative_c(declarePropertyDerivative<std::vector<std::vector<RealGradient> > >(_base_name + "advection_velocity", _c_name )),
   _div_velocity_advection_derivative_c(declarePropertyDerivative<std::vector<std::vector<Real> > >(_base_name + "advection_velocity_divergence", _c_name)),
   _velocity_advection_derivative_eta(declarePropertyDerivative<std::vector<RealGradient> >(_base_name + "advection_velocity", "eta"))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("etas", i);
    _grad_vals[i] = &coupledGradient("etas",i);
  }
}

void
GrainAdvectionVelocity::computeQpProperties()
{
  _velocity_advection[_qp].resize(_op_num);
  _div_velocity_advection[_qp].resize(_op_num);

  unsigned int total_dofs = _subproblem.es().n_dofs();

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    const RealGradient velocity_translation = _mt / _grain_volumes[i] * ((*_vals[i])[_qp] * _grain_forces[i]);
    const Real div_velocity_translation = _mt / _grain_volumes[i] * ((*_grad_vals[i])[_qp] * _grain_forces[i]);
    const RealGradient velocity_rotation = _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    const Real div_velocity_rotation = _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp];

    _velocity_advection[_qp][i] = velocity_translation + velocity_rotation;
    _div_velocity_advection[_qp][i] = div_velocity_translation + div_velocity_rotation;
  }

  if (_fe_problem.currentlyComputingJacobian())
  {
    _velocity_advection_derivative_c[_qp].resize(_op_num);
    _div_velocity_advection_derivative_c[_qp].resize(_op_num);
    _velocity_advection_derivative_eta[_qp].resize(_op_num);
    RealGradient force_c_jacobian = 0.0;
    RealGradient torque_c_jacobian = 0.0;

    for (unsigned int i = 0; i < _op_num; ++i)
    {
      const RealGradient velocity_translation_derivative_eta = _mt / _grain_volumes[i] * _grain_forces[i];
      const RealGradient velocity_rotation_derivative_eta = _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i]));

      _velocity_advection_derivative_eta[_qp][i] = velocity_translation_derivative_eta + velocity_rotation_derivative_eta;

      _velocity_advection_derivative_c[_qp][i].resize(total_dofs);
      _div_velocity_advection_derivative_c[_qp][i].resize(total_dofs);

      for ( unsigned int j = 0; j < total_dofs; ++j)
      {
        force_c_jacobian(0) = _grain_force_derivatives[(6*i+0)*total_dofs+j];
        force_c_jacobian(1) = _grain_force_derivatives[(6*i+1)*total_dofs+j];
        force_c_jacobian(2) = _grain_force_derivatives[(6*i+2)*total_dofs+j];
        torque_c_jacobian(0) = _grain_force_derivatives[(6*i+3)*total_dofs+j];
        torque_c_jacobian(1) = _grain_force_derivatives[(6*i+4)*total_dofs+j];
        torque_c_jacobian(2) = _grain_force_derivatives[(6*i+5)*total_dofs+j];

        const RealGradient velocity_translation_derivative_c = _mt / _grain_volumes[i] * ((*_vals[i])[_qp] * force_c_jacobian);
        const Real div_velocity_translation_derivative_c = _mt / _grain_volumes[i] * ((*_grad_vals[i])[_qp] * force_c_jacobian);
        const RealGradient velocity_rotation_derivative_c = _mr / _grain_volumes[i] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
        const Real div_velocity_rotation_derivative_c = _mr / _grain_volumes[i] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp] ;

        _velocity_advection_derivative_c[_qp][i][j] = velocity_translation_derivative_c + velocity_rotation_derivative_c;
        _div_velocity_advection_derivative_c[_qp][i][j] = div_velocity_translation_derivative_c + div_velocity_rotation_derivative_c;
      }
    }
  }
}
