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
#include "ImplicitStateful.h"

template<>
InputParameters validParams<ImplicitStateful>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("prop_name", "bla");
  params.addRequiredParam<std::string>("coupled_prop_name", "bla");
  params.addRequiredParam<bool>("add_time", "bla");
  params.addRequiredParam<bool>("older", "bla");
  return params;
}

ImplicitStateful::ImplicitStateful(const InputParameters & pars) :
    Material(pars),
    _add_time(pars.get<bool>("add_time")),
    _use_older(pars.get<bool>("older")),
    _prop(declareProperty<Real>(pars.get<std::string>("prop_name"))),
    _coupled_old(getMaterialPropertyOld<Real>(pars.get<std::string>("coupled_prop_name"))),
    _coupled_older(getMaterialPropertyOlder<Real>(pars.get<std::string>("coupled_prop_name")))
{
}

void
ImplicitStateful::computeQpProperties()
{
  if (_fe_problem.timeStep() <= 1)
    _prop[_qp] = 0;

  if (_fe_problem.timeStep() > 0)
  {
    if (_use_older)
      _prop[_qp] = _coupled_older[_qp];
    else
      _prop[_qp] = _coupled_old[_qp];
  }

  if (_add_time)
    _prop[_qp] = _prop[_qp] + _fe_problem.timeStep();
}
