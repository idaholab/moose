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

#include "ExplicitODE.h"

template<>
InputParameters validParams<ExplicitODE>()
{
  InputParameters params = validParams<AuxScalarKernel>();
  params.addParam<Real>("lambda", 1, "Lambda on the right-hand side");
  return params;
}

ExplicitODE::ExplicitODE(const std::string & name, InputParameters parameters) :
    AuxScalarKernel(name, parameters),
    _lambda(getParam<Real>("lambda"))
{
}

ExplicitODE::~ExplicitODE()
{
}

Real
ExplicitODE::computeValue()
{
  return _u_old[_i] * (1 - (_lambda*_dt));
}
