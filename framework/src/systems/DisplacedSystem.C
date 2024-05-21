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
#include "libmesh/default_coupling.h"
#include "libmesh/dof_map.h"

DisplacedSystem::DisplacedSystem(DisplacedProblem & problem,
                                 FEProblemBase & fe_problem,
                                 SystemBase & undisplaced_system,
                                 const std::string & name,
                                 Moose::VarKindType var_kind)
  : SystemBase(problem, fe_problem, name, var_kind),
    _undisplaced_system(undisplaced_system),
    _sys(problem.es().add_system<TransientExplicitSystem>(name))
{
  if (!problem.defaultGhosting())
  {
    auto & dof_map = _sys.get_dof_map();
    dof_map.remove_algebraic_ghosting_functor(dof_map.default_algebraic_ghosting());
    dof_map.set_implicit_neighbor_dofs(false);
  }
}

DisplacedSystem::~DisplacedSystem() {}

NumericVector<Number> &
DisplacedSystem::getVector(const std::string & name)
{
  if (_sys.have_vector(name))
    return _sys.get_vector(name);
  else
    return _undisplaced_system.getVector(name);
}

const NumericVector<Number> &
DisplacedSystem::getVector(const std::string & name) const
{
  if (_sys.have_vector(name))
    return _sys.get_vector(name);
  else
    return _undisplaced_system.getVector(name);
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
