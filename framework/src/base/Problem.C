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

#include "Problem.h"
#include "Factory.h"
#include "Function.h"
#include "TimePeriod.h"

template<>
InputParameters validParams<Problem>()
{
  InputParameters params;
  params.addParam<std::string>("name", "Problem", "The name of the object");
  return params;
}

Problem::Problem(const std::string & name, InputParameters parameters):
  _name(name),
  _pars(parameters),
  _output_initial(false)
{
  unsigned int n_threads = libMesh::n_threads();

  _real_zero.resize(n_threads, 0.);
  _zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _second_zero.resize(n_threads);

  _user_objects.resize(n_threads);
}

Problem::~Problem()
{
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _zero[i].release();
    _grad_zero[i].release();
    _second_zero[i].release();
  }
  _real_zero.release();
  _zero.release();
  _grad_zero.release();
  _second_zero.release();
}

void
Problem::addFunction(std::string /*type*/, const std::string & /*name*/, InputParameters /*parameters*/)
{
}

void
Problem::addUserObject(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    UserObject * uo = static_cast<UserObject *>(Factory::instance()->create(type, name, parameters));
    _user_objects[tid].addUserObject(name, uo);
  }
}

TimePeriod &
Problem::addTimePeriod(const std::string & name, Real start_time)
{
  TimePeriod * tp = new TimePeriod(name, start_time);
  _time_periods.push_back(tp);

  return *tp;
}

TimePeriod *
Problem::getTimePeriodByName(const std::string & name)
{
  for (unsigned int i=0; i<_time_periods.size(); ++i)
    if (_time_periods[i]->name() == name)
      return _time_periods[i];
  return NULL;
}

const std::vector<TimePeriod *> &
Problem::getTimePeriods() const
{
  return _time_periods;
}
