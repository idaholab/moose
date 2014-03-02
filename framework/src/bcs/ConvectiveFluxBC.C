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

#include "ConvectiveFluxBC.h"

template<>
InputParameters validParams<ConvectiveFluxBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.set<Real>("rate")=7500;
  params.set<Real>("initial")=500;
  params.set<Real>("final")=500;
  params.set<Real>("duration")=0.0;
  return params;
}

ConvectiveFluxBC::ConvectiveFluxBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _rate(getParam<Real>("rate")),
    _duration(getParam<Real>("duration"))
{}


Real
ConvectiveFluxBC::computeQpResidual()
{
  Real value;

  if (_t < _duration)
    value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
  else
    value = _final;

  return -(_test[_i][_qp]*_rate*(value - _u[_qp]));
}

Real
ConvectiveFluxBC::computeQpJacobian()
{
  return -(_test[_i][_qp]*_rate*(-_phi[_j][_qp]));
}
