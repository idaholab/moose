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

#include "DirichletPostprocessorBC.h"

template<>
InputParameters validParams<DirichletPostprocessorBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredParam<std::string>("postprocessor_name","The name of postprocessor used to get Value");
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  return params;
}

DirichletPostprocessorBC::DirichletPostprocessorBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),
      _postprocessor_name(getParam<std::string>("postprocessor_name")),
      _value(getPostprocessorValue(_postprocessor_name))
{}

Real
DirichletPostprocessorBC::computeQpResidual()
{
  return _u[_qp]-_value;
}
