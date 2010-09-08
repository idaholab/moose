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

#include "RealPropertyOutput.h"

#include "Material.h"

template<>
InputParameters validParams<RealPropertyOutput>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("prop_name","The Real material property you would like to output");
  return params;
}

RealPropertyOutput::RealPropertyOutput(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _prop_name(getParam<std::string>("prop_name")),
   _prop(getMaterialProperty<Real>(_prop_name))
{}

Real
RealPropertyOutput::computeQpResidual()
{
  return _test[_i][_qp]*(_u[_qp] - _prop[_qp]);
}

Real
RealPropertyOutput::computeQpJacobian()
{
  return _test[_i][_qp]*_test[_j][_qp];
}
