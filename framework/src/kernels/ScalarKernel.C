//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<ScalarKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TransientInterface>();
  params += validParams<TaggingInterface>();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this kernel operates on");

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");

  params.registerBase("ScalarKernel");

  return params;
}

ScalarKernel::ScalarKernel(const InputParameters & parameters)
  : MooseObject(parameters),
    ScalarCoupleable(this),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    TransientInterface(this),
    MeshChangedInterface(parameters),
    // VPPs used by ScalarKernels must be broadcast because we don't know where the
    // ScalarKernel will end up being evaluated
    VectorPostprocessorInterface(this, /*broadcast_by_default=*/true),
    TaggingInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getScalarVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _u_old(_var.slnOld())
{
}

void
ScalarKernel::computeOffDiagJacobian(unsigned int /*jvar*/)
{
}

bool
ScalarKernel::isActive()
{
  return true;
}

MooseVariableScalar &
ScalarKernel::variable()
{
  return _var;
}

SubProblem &
ScalarKernel::subProblem()
{
  return _subproblem;
}
