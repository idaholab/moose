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

#include "SimplePredictor.h"
#include "NonlinearSystem.h"

template<>
InputParameters validParams<SimplePredictor>()
{
  InputParameters params = validParams<Predictor>();
  params.addRequiredParam<Real>("scale", "The scale factor for the predictor (can range from 0 to 1)");

  return params;
}

SimplePredictor::SimplePredictor(const std::string & name, InputParameters parameters) :
    Predictor(name, parameters),
    _scale(getParam<Real>("scale"))
{
}

SimplePredictor::~SimplePredictor()
{
}

void
SimplePredictor::apply(NumericVector<Number> & sln)
{
  if (_dt_old > 0)
  {
    std::streamsize cur_precision(Moose::out.precision());
    Moose::out << "  Applying predictor with scale factor = " << std::fixed << std::setprecision(2) << _scale << std::endl;
    Moose::out << std::scientific << std::setprecision(cur_precision);

    Real dt_adjusted_scale_factor = _scale * _dt / _dt_old;
    if (dt_adjusted_scale_factor != 0.0)
    {
      sln *= (1.0 + dt_adjusted_scale_factor);
      sln.add(-dt_adjusted_scale_factor, _solution_older);
    }
  }
}
