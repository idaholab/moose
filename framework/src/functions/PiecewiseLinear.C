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
  InputParameters params = validParams<Function>();
  params.addParam<std::vector<Real> >("x", "The abscissa values");
  params.addParam<std::vector<Real> >("y", "The ordinate values");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  params.addParam<int>("axis", "The axis used (0, 1, or 2 for x, y, or z) if this is to be a function of position");
  return params;
}

PiecewiseLinear::PiecewiseLinear(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _linear_interp( getParam<std::vector<Real> >("x"),
                  getParam<std::vector<Real> >("y") ),
  _scale_factor( getParam<Real>("scale_factor") ),
  _has_axis(false)
{
  if (parameters.isParamValid("axis"))
  {
    _axis=parameters.get<int>("axis");
    if (_axis < 0 || _axis > 2)
      mooseError("In PiecewiseLinear function axis="<<_axis<<" outside allowable range (0-2).");
    _has_axis = true;
  }
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
    func_value = _linear_interp.sample( p(_axis) );
  }
  else
  {
    func_value = _linear_interp.sample( t );
  }
  return _scale_factor * func_value;
}

Real
PiecewiseLinear::integral()
{
  return _linear_interp.integrate();
}

Real
PiecewiseLinear::average()
{
  return integral()/(_linear_interp.domain(_linear_interp.getSampleSize()-1)-_linear_interp.domain(0));
}
