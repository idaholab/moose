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
  _user_data.resize(n_threads);
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
Problem::addUserData(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    UserData * ud = static_cast<UserData *>(Factory::instance()->create(type, name, parameters));
    _user_data[tid].addUserData(name, ud);
  }
}

const UserData &
Problem::getUserData(const std::string & name, THREAD_ID tid)
{
  UserData * user_data = _user_data[tid].getUserDataByName(name);
  if (user_data == NULL)
  {
    mooseError("Unable to find user data object with name '" + name + "'");
  }
  return *user_data;
}

