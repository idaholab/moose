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
  InputParameters params = validParams<MooseObject>();
  params.addParam<std::vector<Real> >("x", "The abscissa values");
  params.addParam<std::vector<Real> >("y", "The ordinate values");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  return params;
}

PiecewiseLinear::PiecewiseLinear(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _linear_interp( getParam<std::vector<Real> >("x"),
                  getParam<std::vector<Real> >("y") ),
  _scale_factor( getParam<Real>("scale_factor") )
{
}

PiecewiseLinear::~PiecewiseLinear()
{
}

Real
PiecewiseLinear::value(Real t, const Point & /*p*/)
{
  return _scale_factor * _linear_interp.sample( t );
}
