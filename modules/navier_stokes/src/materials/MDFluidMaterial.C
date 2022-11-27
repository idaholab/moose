//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidMaterial.h"
#include "MooseMesh.h"
#include "Steady.h"
#include "libmesh/quadrature.h"
#include "SAMTypes.h"

registerMooseObject("NavierStokesApp", MDFluidMaterial);

InputParameters
MDFluidMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("u", "velocity in x-direction");
  params.addCoupledVar("v", "velocity in y-direction"); // required in 2D/3D
  params.addCoupledVar("w", "velocity in z-direction"); // required in 3D

  params.addRequiredCoupledVar("temperature", "temperature");
  params.addRequiredCoupledVar("pressure", "pressure");
  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
  params.addParam<Real>("scaling_velocity", "Global scaling velocity");

  params.addParam<bool>(
      "compute_turbulence_viscosity", false, "If turbulence viscosity will be computed.");
  params.addParam<std::string>("mixing_length",
                               "Prandtl mixing length to compute turbulence viscosity.");
  params.addCoupledVar("turbulence_viscosity_var",
                       "An aux variable turbulence_viscosity will be computed from.");
  return params;
}

MDFluidMaterial::MDFluidMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mesh_dimension(_mesh.dimension()),
    _u_vel(coupledValue("u")),
    _v_vel(_mesh_dimension >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh_dimension == 3 ? coupledValue("w") : _zero),
    _temperature(coupledValue("temperature")),
    _pressure(coupledValue("pressure")),

    _grad_u(coupledGradient("u")),
    _grad_v(_mesh_dimension >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w(_mesh_dimension == 3 ? coupledGradient("w") : _grad_zero),

    _viscous_stress_tensor(declareProperty<RealTensorValue>("viscous_stress_tensor")),
    _dynamic_viscosity(declareProperty<Real>("dynamic_viscosity")),
    _turbulence_viscosity(declareProperty<Real>("turbulence_viscosity")),
    _k(declareProperty<Real>("k_fluid")),
    _k_turbulence(declareProperty<Real>("k_turbulence")),
    _k_elem(declareProperty<Real>("k_fluid_elem")),
    _cp(declareProperty<Real>("cp_fluid")),
    _rho(declareProperty<Real>("rho_fluid")),
    _hsupg(declareProperty<Real>("hsupg")),
    _tauc(declareProperty<Real>("tauc")),
    _taum(declareProperty<Real>("taum")),
    _taue(declareProperty<Real>("taue")),
    _compute_visc_turbulenc(getParam<bool>("compute_turbulence_viscosity")),
    _has_turb_visc_auxvar(isParamValid("turbulence_viscosity_var")),
    _mixing_length_const(0.0),
    _mixing_length_func(NULL),
    _turb_visc_auxvar(_has_turb_visc_auxvar ? coupledValue("turbulence_viscosity_var") : _zero),
    _has_scale_vel(isParamValid("scaling_velocity")),
    _scaling_velocity(_has_scale_vel ? getParam<Real>("scaling_velocity") : 1.),
    _eos(getUserObject<SinglePhaseFluidProperties>("eos"))
{
  Executioner * myExe = _app.getExecutioner();
  Steady * ss = dynamic_cast<Steady *>(myExe);
  _bSteady = (ss != NULL);

  if (_compute_visc_turbulenc)
  {
    // If visc_turbulence is needed, either 'mixing_length' or 'turbulence_viscosity_var' is needed,
    // but 'mixing_length' and 'turbulence_viscosity_var' cannot be both specified
    if (isParamValid("mixing_length") == isParamValid("turbulence_viscosity_var"))
      mooseError("To compute turbulence viscosity, "
                 "please provide either 'mixing_length' or 'turbulence_viscosity_var'");

    // If 'turbulence_viscosity_var' is not given, 'mixing_length' is expected
    if (!_has_turb_visc_auxvar)
    {
      std::string str = getParam<std::string>("mixing_length");
      if (SAM::isNumber(str))
        _mixing_length_const = SAM::toNumber(str);
      else
        _mixing_length_func = &getFunctionByName(str);
    }
  }
}

void
MDFluidMaterial::computeProperties()
{
  // calculating element average velocity
  _u_elem = 0;
  _v_elem = 0;
  _w_elem = 0;
  //  if(_qrule->n_points()<4) return; // only calculated element-based material properties
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _u_elem += _u_vel[_qp] / _qrule->n_points();
    _v_elem += _v_vel[_qp] / _qrule->n_points();
    _w_elem += _w_vel[_qp] / _qrule->n_points();
  }
  _vel_mag = std::sqrt(_u_elem * _u_elem + _v_elem * _v_elem + _w_elem * _w_elem);

  _k_elem_val = 0.0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    computeQpProperties();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    _k_elem[_qp] = _k_elem_val;
}

void
MDFluidMaterial::computeQpProperties()
{
  compute_fluid_properties();

  // Viscous Stress Tensor
  RealTensorValue grad_vel(_grad_u[_qp], _grad_v[_qp], _grad_w[_qp]);

  grad_vel += grad_vel.transpose();

  // Turbulence viscosity
  _turbulence_viscosity[_qp] = 0.0;
  _k_turbulence[_qp] = 0.0;
  if (_compute_visc_turbulenc)
  {
    if (_has_turb_visc_auxvar)
      _turbulence_viscosity[_qp] = _turb_visc_auxvar[_qp];
    else
    {
      Real strain_rate_mag = 0.0;
      for (unsigned int i = 0; i < 3; i++)
        for (unsigned int j = 0; j < 3; j++)
          strain_rate_mag += 0.5 * grad_vel(i, j) * grad_vel(i, j);

      Real L_mix = (_mixing_length_func == NULL) ? _mixing_length_const
                                                 : _mixing_length_func->value(_t, _q_point[_qp]);

      _turbulence_viscosity[_qp] = _rho[_qp] * L_mix * L_mix * std::sqrt(strain_rate_mag);
    }
    _k_turbulence[_qp] = _cp[_qp] * _turbulence_viscosity[_qp] / 0.9;
  }

  _k_elem_val += (_k[_qp] + _k_turbulence[_qp]) / _qrule->n_points();

  Real div_vel = _grad_u[_qp](0) + _grad_v[_qp](1) + _grad_w[_qp](2);

  // Add diagonal terms
  for (unsigned int i = 0; i < 3; i++)
    grad_vel(i, i) -= 2.0 / 3.0 * div_vel;

  _viscous_stress_tensor[_qp] = (_dynamic_viscosity[_qp] + _turbulence_viscosity[_qp]) * grad_vel;

  // Compute stabilization parameters:
  compute_hsupg();
  compute_tau();
}

void
MDFluidMaterial::compute_fluid_properties()
{
  _dynamic_viscosity[_qp] = _eos.mu_from_p_T(_pressure[_qp], _temperature[_qp]);
  _k[_qp] = _eos.k_from_p_T(_pressure[_qp], _temperature[_qp]);
  _cp[_qp] = _eos.cp_from_p_T(_pressure[_qp], _temperature[_qp]);
  _rho[_qp] = _eos.rho_from_p_T(_pressure[_qp], _temperature[_qp]);
}

void
MDFluidMaterial::compute_hsupg()
{
  // Just use hmin for the element!
  _hsupg[_qp] = _current_elem->hmin();
}

void
MDFluidMaterial::compute_tau()
{
  // element-based stabilization parameters
  Real h2 = _hsupg[_qp] * _hsupg[_qp];
  Real sqrt_term_supg = 4 * _vel_mag * _vel_mag / h2;

  Real sqrt_term_pspg = 0.;
  if (_has_scale_vel)
    sqrt_term_pspg = 4 * _scaling_velocity * _scaling_velocity / h2;
  else
    sqrt_term_pspg = sqrt_term_supg;

  if (!_bSteady)
  {
    sqrt_term_supg += 4. / _dt / _dt;
    sqrt_term_pspg += 4. / _dt / _dt;
  }

  Real vu = (_dynamic_viscosity[_qp] + _turbulence_viscosity[_qp]) / _rho[_qp];
  Real visc_term = 4 * vu / h2;

  Real diffusivity = _k[_qp] / _rho[_qp] / _cp[_qp];
  Real diff_term = 4 * diffusivity / h2;

  _tauc[_qp] = 1. / std::sqrt(sqrt_term_pspg);
  _taum[_qp] = 1. / std::sqrt(sqrt_term_supg + visc_term * visc_term);
  _taue[_qp] = 1. / std::sqrt(sqrt_term_supg + diff_term * diff_term);
}
