//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMaterial.h"
#include "Function.h"
#include "Assembly.h"

registerMooseObject("NavierStokesApp", INSADMaterial);

InputParameters
INSADMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("This is the material class used to compute some of the strong "
                             "residuals for the INS equations.");
  params.addRequiredCoupledVar("velocity", "The velocity");
  params.addRequiredCoupledVar("pressure", "The pressure");
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  params.addParam<bool>("transient_term",
                        true,
                        "Whether there should be a transient term in the momentum residuals.");
  params.addParam<bool>("integrate_p_by_parts", true, "Whether to integrate the pressure by parts");
  params.addParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addParam<FunctionName>("function_x", 0, "The x-velocity mms forcing function.");
  params.addParam<FunctionName>("function_y", 0, "The y-velocity mms forcing function.");
  params.addParam<FunctionName>("function_z", 0, "The z-velocity mms forcing function.");
  return params;
}

INSADMaterial::INSADMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _velocity(adCoupledVectorValue("velocity")),
    _grad_velocity(adCoupledVectorGradient("velocity")),
    _grad_p(adCoupledGradient("pressure")),
    _mu(getADMaterialProperty<Real>("mu_name")),
    _rho(getADMaterialProperty<Real>("rho_name")),
    _transient_term(getParam<bool>("transient_term")),
    _velocity_dot(_transient_term ? &adCoupledVectorDot("velocity") : nullptr),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts")),
    _mass_strong_residual(declareADProperty<Real>("mass_strong_residual")),
    _convective_strong_residual(declareADProperty<RealVectorValue>("convective_strong_residual")),
    _viscous_strong_residual(declareADProperty<RealVectorValue>("viscous_strong_residual")),
    _td_strong_residual(declareADProperty<RealVectorValue>("td_strong_residual")),
    _gravity_strong_residual(declareADProperty<RealVectorValue>("gravity_strong_residual")),
    _mms_function_strong_residual(declareProperty<RealVectorValue>("mms_function_strong_residual")),
    _momentum_strong_residual(declareADProperty<RealVectorValue>("momentum_strong_residual")),
    _x_vel_fn(getFunction("function_x")),
    _y_vel_fn(getFunction("function_y")),
    _z_vel_fn(getFunction("function_z")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _ad_q_point(_bnd ? _assembly.adQPointsFace() : _assembly.adQPoints())
{
  if (parameters.isParamSetByUser("gravity"))
  {
    _gravity_set = true;
    _gravity = getParam<RealVectorValue>("gravity");
  }
  else
    _gravity_set = false;
}

void
INSADMaterial::computeQpProperties()
{
  _mass_strong_residual[_qp] = -_grad_velocity[_qp].tr();
  if (_coord_sys == Moose::COORD_RZ)
    // Subtract u_r / r
    _mass_strong_residual[_qp] -= _velocity[_qp](0) / _q_point[_qp](0);

  _convective_strong_residual[_qp] = _rho[_qp] * _grad_velocity[_qp] * _velocity[_qp];
  _td_strong_residual[_qp] =
      _transient_term ? _rho[_qp] * (*_velocity_dot)[_qp] : ADRealVectorValue(0);
  _gravity_strong_residual[_qp] = _gravity_set ? -_rho[_qp] * _gravity : ADRealVectorValue(0);
  _mms_function_strong_residual[_qp] = -RealVectorValue(_x_vel_fn.value(_t, _q_point[_qp]),
                                                        _y_vel_fn.value(_t, _q_point[_qp]),
                                                        _z_vel_fn.value(_t, _q_point[_qp]));

  // // The code immediately below is fictional. E.g. there is no Moose::Laplacian nor is there
  // // currently a _second_velocity member because TypeNTensor (where N = 3 in this case) math is not
  // // really implemented in libMesh. Hence we cannot add this strong form contribution of the viscous
  // // term at this time. Note that for linear elements this introduces no error in the consistency of
  // // stabilization methods, and in general for bi-linear elements, the error introduced is small
  // _viscous_strong_residual[_qp] = -_mu[_qp] * Moose::Laplacian(_second_velocity[_qp]);

  if (_coord_sys == Moose::COORD_RZ)
  {
    // To understand the code immediately below, visit
    // https://en.wikipedia.org/wiki/Del_in_cylindrical_and_spherical_coordinates.
    // The u_r / r^2 term comes from the vector Laplacian. The -du_i/dr * 1/r term comes from
    // the scalar Laplacian. The scalar Laplacian in axisymmetric cylindrical coordinates is
    // equivalent to the Cartesian Laplacian plus a 1/r * du_i/dr term. And of course we are
    // applying a minus sign here because the strong form is -\nabala^2 * \vec{u}
    //
    // Another note: libMesh implements grad(v) as dvi/dxj

    if (_use_displaced_mesh)
      _viscous_strong_residual[_qp] = ADRealVectorValue(
          // u_r
          // Additional term from vector Laplacian
          _mu[_qp] * (_velocity[_qp](0) / (_ad_q_point[_qp](0) * _ad_q_point[_qp](0)) -
                      // Additional term from scalar Laplacian
                      _grad_velocity[_qp](0, 0) / _ad_q_point[_qp](0)),
          // u_z
          // Additional term from scalar Laplacian
          -_mu[_qp] * _grad_velocity[_qp](1, 0) / _ad_q_point[_qp](0),
          0);
    else
      _viscous_strong_residual[_qp] =
          // u_r
          // Additional term from vector Laplacian
          ADRealVectorValue(_mu[_qp] * (_velocity[_qp](0) / (_q_point[_qp](0) * _q_point[_qp](0)) -
                                        // Additional term from scalar Laplacian
                                        _grad_velocity[_qp](0, 0) / _q_point[_qp](0)),
                            // u_z
                            // Additional term from scalar Laplacian
                            -_mu[_qp] * _grad_velocity[_qp](1, 0) / _q_point[_qp](0),
                            0);
  }

  _momentum_strong_residual[_qp] =
      _gravity_strong_residual[_qp] + _mms_function_strong_residual[_qp] +
      _convective_strong_residual[_qp] + _td_strong_residual[_qp] + _grad_p[_qp];

  if (_coord_sys == Moose::COORD_RZ)
    _momentum_strong_residual[_qp] += _viscous_strong_residual[_qp];
}
