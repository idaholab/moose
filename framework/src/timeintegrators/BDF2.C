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

#include "BDF2.h"
#include "NonlinearSystem.h"

template<>
InputParameters validParams<BDF2>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

BDF2::BDF2(const std::string & name, InputParameters parameters) :
    TimeIntegrator(name, parameters),
    _weight(3, 0.)
{
}

BDF2::~BDF2()
{
}

void
BDF2::preStep()
{
  Real sum = _dt + _dt_old;
  _weight[0] =  1. + _dt / sum;
  _weight[1] = -sum / _dt_old;
  _weight[2] = _dt * _dt / _dt_old / sum;

}

void
BDF2::computeTimeDerivatives()
{
  if (_t_step == 1)
  {
    _u_dot  = *_nl.currentSolution();
    _u_dot -= _nl.solutionOld();
    _u_dot *= 1 / _dt;
    _u_dot.close();

    _du_dot_du = 1.0 / _dt;
    _du_dot_du.close();
  }
  else
  {
    _u_dot.zero();
    _u_dot.add(_weight[0], *_nl.currentSolution());
    _u_dot.add(_weight[1], _nl.solutionOld());
    _u_dot.add(_weight[2], _nl.solutionOlder());
    _u_dot.scale(1. / _dt);
    _u_dot.close();

    _du_dot_du = _weight[0] / _dt;
    _du_dot_du.close();
  }
}

void
BDF2::postStep(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}
