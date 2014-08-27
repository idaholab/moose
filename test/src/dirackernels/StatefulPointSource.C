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

#include "StatefulPointSource.h"

template<>
InputParameters validParams<StatefulPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point");
  return params;
}

StatefulPointSource::StatefulPointSource(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),
    _p(getParam<Point>("point")),
    _value(getMaterialPropertyOld<Real>("thermal_conductivity"))
{
}

void
StatefulPointSource::addPoints()
{
  addPoint(_p);
}

Real
StatefulPointSource::computeQpResidual()
{
  _console << "Thermal conductivity=" << _value[_qp] << std::endl;

  // This is negative because it's a forcing function that has been
  // brought over to the left side.
  return -_test[_i][_qp]*_value[_qp];
}
