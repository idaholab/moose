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

#include "MaterialPointSource.h"

template<>
InputParameters validParams<MaterialPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point");
  return params;
}

MaterialPointSource::MaterialPointSource(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),
    _p(getParam<Point>("point")),
    _value(getMaterialProperty<Real>("matp"))
{
}

void
MaterialPointSource::addPoints()
{
  addPoint(_p);
}

Real
MaterialPointSource::computeQpResidual()
{
  // These values should match... this shows the problem
  // Moose::out << "_value[_qp]=" << _value[_qp] << std::endl;
  // Moose::out << "_q_point[_qp](0)=" << _q_point[_qp](0) << std::endl;

  // This is negative because it's a forcing function that has been
  // brought over to the left side.
  return -_test[_i][_qp]*_value[_qp];
}

