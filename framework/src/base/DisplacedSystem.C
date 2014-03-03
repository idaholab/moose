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

#include "DisplacedSystem.h"
#include "DisplacedProblem.h"

DisplacedSystem::DisplacedSystem(DisplacedProblem & problem, SystemBase & undisplaced_system, const std::string & name, Moose::VarKindType var_kind) :
    SystemTempl<TransientExplicitSystem>(problem, name, var_kind),
    _undisplaced_system(undisplaced_system)
{
}

DisplacedSystem::~DisplacedSystem()
{
}

void
DisplacedSystem::init()
{
}

NumericVector<Number> &
DisplacedSystem::getVector(std::string name)
{
  if (_sys.have_vector(name))
    return _sys.get_vector(name);
  else
    return _undisplaced_system.getVector(name);
}

