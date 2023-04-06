//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "INSADStabilized3Eqn.h"

registerMooseObject("NavierStokesApp", INSADStabilized3Eqn);

InputParameters
INSADStabilized3Eqn::validParams()
{
  InputParameters params = INSADTauMaterialTempl<INSAD3Eqn>::validParams();
  params.addClassDescription("This is the material class used to compute the stabilization "
                             "parameter tau for momentum and tau_energy for the energy equation.");
  params.addParam<MaterialPropertyName>(
      "k_name", "k", "the name of the thermal conductivity material property");
  params.addParam<MaterialPropertyName>(
      "grad_k_name",
      "grad_k",
      "the name of the gradient of the thermal conductivity material property");
  return params;
}

INSADStabilized3Eqn::INSADStabilized3Eqn(const InputParameters & parameters)
  : INSADTauMaterialTempl<INSAD3Eqn>(parameters),
    _second_temperature(adCoupledSecond("temperature")),
    _k(getADMaterialProperty<Real>("k_name")),
    _grad_k(hasADMaterialProperty<RealVectorValue>("grad_k_name")
                ? &getADMaterialProperty<RealVectorValue>("grad_k_name")
                : nullptr),
    _tau_energy(declareADProperty<Real>("tau_energy")),
    _temperature_strong_residual(declareADProperty<Real>("temperature_strong_residual"))
{
}

void
INSADStabilized3Eqn::computeQpProperties()
{
  INSADTauMaterialTempl<INSAD3Eqn>::computeQpProperties();

  const auto dissipation_coefficient = _k[_qp] / (_rho[_qp] * _cp[_qp]);
  const auto transient_part = _has_energy_transient ? 4. / (_dt * _dt) : 0.;
  const auto speed = NS::computeSpeed(_velocity[_qp]);
  _tau_energy[_qp] =
      _alpha / std::sqrt(transient_part + (2. * speed / _hmax) * (2. * speed / _hmax) +
                         9. * (4. * dissipation_coefficient / (_hmax * _hmax)) *
                             (4. * dissipation_coefficient / (_hmax * _hmax)));

  // Start with the conductive term
  _temperature_strong_residual[_qp] = -_k[_qp] * _second_temperature[_qp].tr();
  if (_grad_k)
    _temperature_strong_residual[_qp] -= (*_grad_k)[_qp] * _grad_temperature[_qp];

  // advective
  _temperature_strong_residual[_qp] += _temperature_advective_strong_residual[_qp];

  if (_has_energy_transient)
    _temperature_strong_residual[_qp] += _temperature_td_strong_residual[_qp];

  if (_has_ambient_convection)
    _temperature_strong_residual[_qp] += _temperature_ambient_convection_strong_residual[_qp];

  if (_has_heat_source)
    _temperature_strong_residual[_qp] += _temperature_source_strong_residual[_qp];
}
