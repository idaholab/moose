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

  _functions.resize(n_threads);
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

  for (unsigned int i = 0; i < n_threads; i++)
    for (std::map<std::string, Function *>::iterator it = _functions[i].begin(); it != _functions[i].end(); ++it)
      delete it->second;
}

void
Problem::addFunction(std::string type, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    Function * func = static_cast<Function *>(Factory::instance()->create(type, name, parameters));
    _functions[tid][name] = func;
  }
}

Function &
Problem::getFunction(const std::string & name, THREAD_ID tid)
{
  Function * function = _functions[tid][name];
  if (!function)
  {
    mooseError("Unable to find function " + name);
  }
  return *function;
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

const UserObject &
Problem::getUserObject(const std::string & name, THREAD_ID tid)
{
  UserObject * user_object = _user_objects[tid].getUserObjectByName(name);
  if (user_object == NULL)
  {
    mooseError("Unable to find user object with name '" + name + "'");
  }
  return *user_object;
}

void
Problem::addTimePeriod(const std::string & name, Real start_time, Real end_time)
{
  TimePeriod * tp = new TimePeriod;
  tp->_start = start_time;
  tp->_end = end_time;
  _time_periods[name] = tp;
}

TimePeriod *
Problem::getTimePeriodByName(const std::string & name)
{
  std::map<std::string, TimePeriod *>::iterator it = _time_periods.find(name);
  if (it == _time_periods.end())
    return NULL;
  else
    return it->second;
}
