//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearTimeIntegrator.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"

#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/dof_map.h"

NonlinearTimeIntegrator::NonlinearTimeIntegrator(FEProblemBase & problem, SystemBase & system)
  : _nl(dynamic_cast<NonlinearSystemBase *>(&system)),
    _integrates_nl(_nl),
    _nonlinear_implicit_system(_integrates_nl ? dynamic_cast<NonlinearImplicitSystem *>(&_nl->system()) : nullptr),
    _Re_time(_integrates_nl ? &_nl->getResidualTimeVector() : nullptr),
    _Re_non_time(_integrates_nl ? &_nl->getResidualNonTimeVector() : nullptr),
    _u_dot_factor_tag(problem.addVectorTag("u_dot_factor", Moose::VECTOR_TAG_SOLUTION)),
    _u_dotdot_factor_tag(problem.addVectorTag("u_dotdot_factor", Moose::VECTOR_TAG_SOLUTION))
{
}

NumericVector<Number> * NonlinearTimeIntegrator::addVectorForNonlinearTI(const std::string & name,
                                                  const bool project,
                                                  const ParallelType type)
{
  if (_integrates_nl)
    return &_nl->addVector(name, project, type);
  else
    return nullptr;
}
