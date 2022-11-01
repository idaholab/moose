//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Indicator.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/threads.h"

InputParameters
Indicator::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += BlockRestrictable::validParams();
  params += OutputInterface::validParams();
  params += MaterialPropertyInterface::validParams();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("Indicator");

  return params;
}

Indicator::Indicator(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    MooseVariableDependencyInterface(this),
    Restartable(this, "Indicators"),
    OutputInterface(parameters),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _solution(_sys.solution()),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, 0)),
    _mesh(_subproblem.mesh())
{
}
