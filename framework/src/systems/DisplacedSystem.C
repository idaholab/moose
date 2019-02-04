//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplacedSystem.h"
#include "DisplacedProblem.h"

#include "libmesh/transient_system.h"
#include "libmesh/explicit_system.h"

DisplacedSystem::DisplacedSystem(DisplacedProblem & problem,
                                 SystemBase & undisplaced_system,
                                 const std::string & name,
                                 Moose::VarKindType var_kind)
  : SystemBase(problem, name, var_kind),
    _undisplaced_system(undisplaced_system),
    _sys(problem.es().add_system<TransientExplicitSystem>(name))
{
}

DisplacedSystem::~DisplacedSystem() {}

void
DisplacedSystem::init()
{
}

NumericVector<Number> &
DisplacedSystem::getVector(const std::string & name)
{
  if (_sys.have_vector(name))
    return _sys.get_vector(name);
  else
    return _undisplaced_system.getVector(name);
}

void
DisplacedSystem::addTimeIntegrator(std::shared_ptr<TimeIntegrator> ti)
{
  _time_integrator = ti;
}

NumericVector<Number> &
DisplacedSystem::solutionOld()
{
  return *_sys.old_local_solution;
}

NumericVector<Number> &
DisplacedSystem::solutionOlder()
{
  return *_sys.older_local_solution;
}

System &
DisplacedSystem::system()
{
  return _sys;
}

const System &
DisplacedSystem::system() const
{
  return _sys;
}
