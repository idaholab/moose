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
#include "CoupledTransientExecutioner.h"
#include "CoupledProblem.h"
#include "Transient.h"
#include "TimeStepper.h"

template<>
InputParameters validParams<CoupledTransientExecutioner>()
{
  InputParameters params = validParams<CoupledExecutioner>();
  params.addParam<unsigned int>("num_steps",       std::numeric_limits<unsigned int>::max(),     "The number of timesteps in a transient run");
  return params;
}

CoupledTransientExecutioner::CoupledTransientExecutioner(const std::string & name, InputParameters parameters) :
    CoupledExecutioner(name, parameters),
    _time(0),
    _dt(0),
    _t_step(0),
    _n_steps(getParam<unsigned int>("num_steps"))
{
}

CoupledTransientExecutioner::~CoupledTransientExecutioner()
{
}

void
CoupledTransientExecutioner::execute()
{
  if (_executioners.size() != _fe_problems.size())
    mooseError("The number of executioners of different that the number of problems.");

  unsigned int n_problems = _executioners.size();
  // preExecute
  for (unsigned int i = 0; i < n_problems; i++)
  {
    _executioners[i]->init();
    _executioners[i]->preExecute();
  }

  std::vector<Transient *> trans(n_problems);
  for (unsigned int i = 0; i < n_problems; i++)
  {
    Transient * exec = dynamic_cast<Transient *>(_executioners[i]);
    if (exec == NULL)
      mooseError("Executioner for problem '" << _fe_problems[i]->name() << "' has to be of a transient type.");
    trans[i] = exec;
  }

  bool first = true;

  for (_t_step = 0; _t_step < _n_steps; _t_step++)
  {
    if(first != true)
    {
      for (unsigned int i = 0; i < n_problems; i++)
        trans[i]->incrementStepOrReject();
    }

    first = false;

    for (unsigned int i = 0; i < n_problems; i++)
      trans[i]->computeDT();

    _dt = trans[0]->getDT();
    _time += _dt;

    for (unsigned int i = 0; i < n_problems; i++)
    {
      std::cout << "Solving '" << _fep_mapping[_fe_problems[i]] << "'" << std::endl;
      trans[i]->takeStep(_dt);
      projectVariables(*_fe_problems[(i + 1) % n_problems]);
    }

    for (unsigned int i = 0; i < n_problems; i++)
      trans[i]->endStep();
  }
}
