/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "RichardsHalfGaussianSink.h"

#include "Material.h"

#include <iostream>


template<>
InputParameters validParams<RichardsHalfGaussianSink>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<Real>("max", "Maximum of the flux (measured in kg.m^-2.s^-1).  Flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and Flux out = max for p>centre.  Note, to make this a source rather than a sink, let max<0");
  params.addRequiredParam<Real>("sd", "Standard deviation of the Gaussian (measured in Pa).  Flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and Flux out = max for p>centre.");
  params.addRequiredParam<Real>("centre", "Centre of the Gaussian (measured in Pa).  Flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and Flux out = max for p>centre.");
  return params;
}

RichardsHalfGaussianSink::RichardsHalfGaussianSink(const std::string & name,
                                             InputParameters parameters) :
    IntegratedBC(name,parameters),
    _maximum(getParam<Real>("max")),
    _sd(getParam<Real>("sd")),
    _centre(getParam<Real>("centre")),
    _vel_SUPG(getMaterialProperty<RealVectorValue>("vel_SUPG")),
    _vel_prime_SUPG(getMaterialProperty<RealTensorValue>("vel_prime_SUPG")),
    _tau_SUPG(getMaterialProperty<Real>("tau_SUPG")),
    _tau_prime_SUPG(getMaterialProperty<RealVectorValue>("tau_prime_SUPG"))
{}

Real
RichardsHalfGaussianSink::computeQpResidual()
{
  Real test_fcn = _test[_i][_qp] + _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp]*0;
  if (_u[_qp] >= _centre) {
    return test_fcn*_maximum;
  }
  else {
    return test_fcn*_maximum*exp(-0.5*std::pow((_u[_qp] - _centre)/_sd, 2));
  }
}

Real
RichardsHalfGaussianSink::computeQpJacobian()
{
  Real test_fcn = _test[_i][_qp] + _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp]*0;
  Real supg_test_prime = ((_tau_prime_SUPG[_qp]*_grad_phi[_j][_qp])*(_vel_SUPG[_qp]*_grad_test[_i][_qp]) + _tau_SUPG[_qp]*(_vel_prime_SUPG[_qp]*_grad_phi[_j][_qp])*_grad_test[_i][_qp])*0;
  if (_u[_qp] >= _centre) {
    return supg_test_prime*_maximum;
  }
  else {
    Real supg_term = supg_test_prime*_maximum*exp(-0.5*std::pow((_u[_qp] - _centre)/_sd, 2));
    return -test_fcn*_maximum*(_u[_qp] - _centre)/std::pow(_sd, 2)*exp(-0.5*std::pow((_u[_qp] - _centre)/_sd, 2))*_phi[_j][_qp] + supg_term;
  }
}
