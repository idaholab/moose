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

template <>
InputParameters
validParams<SimplePredictor>()
{
  InputParameters params = validParams<Predictor>();

  return params;
}

SimplePredictor::SimplePredictor(const InputParameters & parameters) : Predictor(parameters) {}

bool
SimplePredictor::shouldApply()
{
  bool should_apply = true;
  should_apply = Predictor::shouldApply();

  if (_t_step < 2 || _dt_old <= 0)
    should_apply = false;

  if (!should_apply)
    _console << "  Skipping predictor this step" << std::endl;

  return should_apply;
}

void
SimplePredictor::apply(NumericVector<Number> & sln)
{
  // Save the original stream flags
  std::ios_base::fmtflags out_flags = Moose::out.flags();

  _console << "  Applying predictor with scale factor = " << std::fixed << std::setprecision(2)
           << _scale << std::endl;

  // Restore the flags
  Moose::out.flags(out_flags);

  Real dt_adjusted_scale_factor = _scale * _dt / _dt_old;
  if (dt_adjusted_scale_factor != 0.0)
  {
    sln *= (1.0 + dt_adjusted_scale_factor);
    sln.add(-dt_adjusted_scale_factor, _solution_older);
  }
}
