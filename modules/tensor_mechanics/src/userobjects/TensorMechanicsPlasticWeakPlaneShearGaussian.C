#include "TensorMechanicsPlasticWeakPlaneShearGaussian.h"
#include <math.h> // for M_PI

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneShearGaussian>()
{
  InputParameters params = validParams<TensorMechanicsPlasticWeakPlaneShear>();
  params.addClassDescription("Non-associative finite-strain weak-plane shear plasticity with hardening/softening governed by a Gaussian rule");

  return params;
}

TensorMechanicsPlasticWeakPlaneShearGaussian::TensorMechanicsPlasticWeakPlaneShearGaussian(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticWeakPlaneShear(name, parameters)
{
}


Real
TensorMechanicsPlasticWeakPlaneShearGaussian::cohesion(const Real internal_param) const
{
  if (internal_param >= 0)
    return _cohesion_residual + (_cohesion - _cohesion_residual)*std::exp(-std::pow(_cohesion_rate*internal_param, 2));
  else
    return _cohesion;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::dcohesion(const Real internal_param) const
{
  if (internal_param >= 0)
    return -2.0*std::pow(_cohesion_rate, 2)*internal_param*(_cohesion - _cohesion_residual)*std::exp(-std::pow(_cohesion_rate*internal_param, 2));
  else
    return 0.0;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::tan_phi(const Real internal_param) const
{
  if (internal_param >= 0)
    return _tan_phi_residual + (_tan_phi - _tan_phi_residual)*std::exp(-std::pow(_tan_phi_rate*internal_param, 2));
  else
    return _tan_phi;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::dtan_phi(const Real internal_param) const
{
  if (internal_param >= 0)
    return -2*std::pow(_tan_phi_rate, 2)*internal_param*(_tan_phi - _tan_phi_residual)*std::exp(-std::pow(_tan_phi_rate*internal_param, 2));
  else
    return 0.0;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::tan_psi(const Real internal_param) const
{
  if (internal_param >= 0)
    return _tan_psi_residual + (_tan_psi - _tan_psi_residual)*std::exp(-std::pow(_tan_psi_rate*internal_param, 2));
  else
    return _tan_psi;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::dtan_psi(const Real internal_param) const
{
  if (internal_param >= 0)
    return -2*std::pow(_tan_psi_rate, 2)*internal_param*(_tan_psi - _tan_psi_residual)*std::exp(-std::pow(_tan_psi_rate*internal_param, 2));
  else
    return 0.0;
}
