//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMaterial);

InputParameters
INSFVMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("This is the material class used to compute advected quantities for "
                             "the finite-volume implementation of the Navier-Stokes equations.");
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  params.addRequiredCoupledVar("pressure", "The pressure variable.");
  params.addCoupledVar("temperature", "the temperature");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "the name of the specific heat capacity");
  return params;
}

INSFVMaterial::INSFVMaterial(const InputParameters & parameters)
  : Material(parameters),
    _u_vel(adCoupledValue("u")),
    _v_vel(adCoupledValue("v")),
    _w_vel(adCoupledValue("w")),
    _p_var(adCoupledValue("pressure")),
    _velocity(declareADProperty<RealVectorValue>(NS::velocity)),
    _rho_u(declareADProperty<Real>(NS::momentum_x)),
    _rho_v(declareADProperty<Real>(NS::momentum_y)),
    _rho_w(declareADProperty<Real>(NS::momentum_z)),
    _p(declareADProperty<Real>(NS::pressure)),
    _rho(getParam<Real>("rho")),
    _has_temperature(isParamValid("temperature")),
    _temperature(_has_temperature ? &adCoupledValue("temperature") : nullptr),
    _cp(_has_temperature ? &getADMaterialProperty<Real>("cp_name") : nullptr),
    _rho_cp_temp(_has_temperature ? &declareADProperty<Real>("rho_cp_temp") : nullptr)
{
}

void
INSFVMaterial::computeQpProperties()
{
  _p[_qp] = _p_var[_qp];
  _velocity[_qp](0) = _u_vel[_qp];
  _rho_u[_qp] = _rho * _u_vel[_qp];

  if (_mesh.dimension() >= 2)
  {
    _velocity[_qp](1) = _v_vel[_qp];
    _rho_v[_qp] = _rho * _v_vel[_qp];
  }

  if (_mesh.dimension() >= 3)
  {
    mooseAssert(_mesh.dimension() == 3, "The mesh dimension is greater than 3?!");

    _velocity[_qp](2) = _w_vel[_qp];
    _rho_w[_qp] = _rho * _w_vel[_qp];
  }

  mooseAssert(_mesh.dimension() >= 3 || _velocity[_qp](2) == 0,
              "z-velocity component should be zero");
  mooseAssert(_mesh.dimension() >= 2 || _velocity[_qp](1) == 0,
              "y-velocity component should be zero");

  if (_has_temperature)
    (*_rho_cp_temp)[_qp] = _rho * (*_cp)[_qp] * (*_temperature)[_qp];
}
