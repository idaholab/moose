//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMClosureBase.h"
#include "MooseObject.h"
#include "SCM.h"

InputParameters
SCMClosureBase::validParams()
{
  InputParameters params = ThreadedGeneralUserObject::validParams();
  params.registerBase("SCMClosureBase");

  // Suppress unused parameters
  params.suppressParameter<bool>("use_displaced_mesh");
  params.suppressParameter<ExecFlagEnum>("execute_on");
  params.suppressParameter<bool>("allow_duplicate_execution_on_initial");
  params.suppressParameter<bool>("force_preic");
  params.suppressParameter<bool>("force_preaux");
  params.suppressParameter<bool>("force_postaux");
  params.suppressParameter<int>("execution_order_group");

  return params;
}

SCMClosureBase::SCMClosureBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _subchannel_mesh(SCM::getConstMesh<SubChannelMesh>(_subproblem.mesh())),
    _scm_problem(dynamic_cast<SubChannel1PhaseProblem *>(&_fe_problem))
{
  if (!_scm_problem)
    mooseError(
        name(), ": SCM closures require SubChannel1PhaseProblem (got ", _fe_problem.name(), ").");
}
