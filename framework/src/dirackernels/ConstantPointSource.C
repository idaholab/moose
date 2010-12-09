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

#include "ConstantPointSource.h"

// Moose includes
#include "MooseSystem.h"

template<>
InputParameters validParams<ConstantPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Real>("value", "The value of the point source");
  params.addRequiredParam<std::vector<Real> >("point", "The x,y,z coordinates of the point");
  return params;
}

ConstantPointSource::ConstantPointSource(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _value(getParam<Real>("value")),
   _point_param(getParam<std::vector<Real> >("point"))
{
  _p(0) = _point_param[0];

  if(_point_param.size() > 1)
  {
    _p(1) = _point_param[1];

    if(_point_param.size() > 2)
    {
      _p(2) = _point_param[2];
    }
  }
}
           
void
ConstantPointSource::addPoints()
{
  addPoint(_p);
}

Real
ConstantPointSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp]*_value;
}

