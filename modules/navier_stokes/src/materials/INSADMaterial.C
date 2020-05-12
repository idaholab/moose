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
#include "INSADObjectTracker.h"

registerMooseObject("NavierStokesApp", INSADMaterial);

const INSADObjectTracker * INSADMaterial::_object_tracker = nullptr;

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
  return params;
}

INSADMaterial::INSADMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _velocity(adCoupledVectorValue("velocity")),
    _grad_velocity(adCoupledVectorGradient("velocity")),
    _grad_p(adCoupledGradient("pressure")),
    _mu(getADMaterialProperty<Real>("mu_name")),
    _rho(getADMaterialProperty<Real>("rho_name")),
    _velocity_dot(nullptr),
    _mass_strong_residual(declareADProperty<Real>("mass_strong_residual")),
    _convective_strong_residual(declareADProperty<RealVectorValue>("convective_strong_residual")),
    _viscous_strong_residual(declareADProperty<RealVectorValue>("viscous_strong_residual")),
    // We have to declare the below strong residuals for integrity check purposes even though we may
    // not compute them. This may incur some unnecessary cost for a non-sparse derivative container
    // since when the properties are resized the entire non-sparse derivative containers will be
    // initialized to zero
    _td_strong_residual(declareADProperty<RealVectorValue>("td_strong_residual")),
    _gravity_strong_residual(declareADProperty<RealVectorValue>("gravity_strong_residual")),
    _boussinesq_strong_residual(declareADProperty<RealVectorValue>("boussinesq_strong_residual")),
    // _mms_function_strong_residual(declareProperty<RealVectorValue>("mms_function_strong_residual")),
    _momentum_strong_residual(declareADProperty<RealVectorValue>("momentum_strong_residual")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _ad_q_point(_bnd ? _assembly.adQPointsFace() : _assembly.adQPoints())
{
  if (!_object_tracker)
  {
    InputParameters tracker_params = INSADObjectTracker::validParams();
    tracker_params.addPrivateParam("_moose_app", &_app);

    _fe_problem.addUserObject("INSADObjectTracker", "ins_ad_object_tracker", tracker_params);

    // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
    // don't need
    _object_tracker = &_fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker");
  }
}

void
INSADMaterial::initialSetup()
{
  if (_object_tracker->hasTransient())
    _velocity_dot = &adCoupledVectorDot("velocity");
}

void
INSADMaterial::computeQpProperties()
{
  _mass_strong_residual[_qp] = -_grad_velocity[_qp].tr();
  if (_coord_sys == Moose::COORD_RZ)
    // Subtract u_r / r
    _mass_strong_residual[_qp] -=
        _velocity[_qp](0) / (_use_displaced_mesh ? _ad_q_point[_qp](0) : _q_point[_qp](0));

  _convective_strong_residual[_qp] = _rho[_qp] * _grad_velocity[_qp] * _velocity[_qp];
  if (_object_tracker->hasTransient())
    _td_strong_residual[_qp] = _rho[_qp] * (*_velocity_dot)[_qp];
  if (_object_tracker->hasGravity())
    _gravity_strong_residual[_qp] = -_rho[_qp] * _object_tracker->gravityVector();
  if (_object_tracker->hasBoussinesq())
    _boussinesq_strong_residual[_qp] =
        (*_object_tracker->alpha())[_qp] * _object_tracker->gravityVector() * _rho[_qp] *
        ((*_object_tracker->t())[_qp] - (*_object_tracker->tRef())[_qp]);

  // // Future Addition
  // _mms_function_strong_residual[_qp] = -RealVectorValue(_x_vel_fn.value(_t, _q_point[_qp]),
  //                                                       _y_vel_fn.value(_t, _q_point[_qp]),
  //                                                       _z_vel_fn.value(_t, _q_point[_qp]));

  // // The code immediately below is fictional. E.g. there is no Moose::Laplacian nor is there
  // // currently a _second_velocity member because TypeNTensor (where N = 3 in this case) math is not
  // // really implemented in libMesh. Hence we cannot add this strong form contribution of the viscous
  // // term at this time. Note that for linear elements this introduces no error in the consistency of
  // // stabilization methods, and in general for bi-linear elements, the error introduced is small
  // _viscous_strong_residual[_qp] = -_mu[_qp] * Moose::Laplacian(_second_velocity[_qp]);

  if (_coord_sys == Moose::COORD_RZ)
    viscousTermRZ();

  _momentum_strong_residual[_qp] = _convective_strong_residual[_qp] + _grad_p[_qp];

  // Since we can't current compute vector Laplacians we only have strong residual contributions
  // from the viscous term in the RZ coordinate system
  if (_coord_sys == Moose::COORD_RZ)
    _momentum_strong_residual[_qp] += _viscous_strong_residual[_qp];

  if (_object_tracker->hasTransient())
    _momentum_strong_residual[_qp] += _td_strong_residual[_qp];

  if (_object_tracker->hasGravity())
    _momentum_strong_residual[_qp] += _gravity_strong_residual[_qp];

  if (_object_tracker->hasBoussinesq())
    _momentum_strong_residual[_qp] += _boussinesq_strong_residual[_qp];

  // // Future addition
  // if (_object_tracker->hasMMS())
  //   _momentum_strong_residual[_qp] += _mms_function_strong_residual[_qp];
}

void
INSADMaterial::viscousTermRZ()
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
  {
    ADReal r = _ad_q_point[_qp](0);

    if (_object_tracker->viscousForm() == "laplace")
      _viscous_strong_residual[_qp] = ADRealVectorValue(
          // u_r
          // Additional term from vector Laplacian
          _mu[_qp] * (_velocity[_qp](0) / (r * r) -
                      // Additional term from scalar Laplacian
                      _grad_velocity[_qp](0, 0) / r),
          // u_z
          // Additional term from scalar Laplacian
          -_mu[_qp] * _grad_velocity[_qp](1, 0) / r,
          0);
    else
      _viscous_strong_residual[_qp] = ADRealVectorValue(
          2. * _mu[_qp] * (_velocity[_qp](0) / (r * r) - _grad_velocity[_qp](0, 0) / r),
          -_mu[_qp] / r * (_grad_velocity[_qp](1, 0) + _grad_velocity[_qp](0, 1)),
          0);
  }
  else
  {
    Real r = _q_point[_qp](0);
    if (_object_tracker->viscousForm() == "laplace")
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
    else
      _viscous_strong_residual[_qp] = ADRealVectorValue(
          2. * _mu[_qp] * (_velocity[_qp](0) / (r * r) - _grad_velocity[_qp](0, 0) / r),
          -_mu[_qp] / r * (_grad_velocity[_qp](1, 0) + _grad_velocity[_qp](0, 1)),
          0);
  }
}
