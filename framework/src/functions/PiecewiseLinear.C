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

#include "PiecewiseLinear.h"

template<>
InputParameters validParams<PiecewiseLinear>()
{
  InputParameters params = validParams<Piecewise>();
  return params;
}

PiecewiseLinear::PiecewiseLinear(const std::string & name, InputParameters parameters) :
  Piecewise(name, parameters)
{
}

PiecewiseLinear::~PiecewiseLinear()
{
}

Real
PiecewiseLinear::value(Real t, const Point & p)
{
  Real func_value;
  if (_has_axis)
  {
    func_value = _linear_interp->sample( p(_axis) );
  }
  else
  {
    func_value = _linear_interp->sample( t );
  }
  return _scale_factor * func_value;
}

Real
PiecewiseLinear::timeDerivative(Real t, const Point & p)
{
  Real func_value;
  if (_has_axis)
  {
    func_value = _linear_interp->sampleDerivative( p(_axis) );
  }
  else
  {
    func_value = _linear_interp->sampleDerivative( t );
  }
  return _scale_factor * func_value;
}

Real
PiecewiseLinear::integral()
{
  return _scale_factor * _linear_interp->integrate();
}

Real
PiecewiseLinear::average()
{
  return integral()/(_linear_interp->domain(_linear_interp->getSampleSize()-1)-_linear_interp->domain(0));
}
