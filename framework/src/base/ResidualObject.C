//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ResidualObject.h"
#include "SubProblem.h"
#include "InputParameters.h"

InputParameters
ResidualObject::validParams()
{
  auto params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += RandomInterface::validParams();
  params += MeshChangedInterface::validParams();
  params += TaggingInterface::validParams();

  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this residual object operates on");

  params.declareControllable("enable");
  params.set<bool>("_residual_object") = true;
  return params;
}

ResidualObject::ResidualObject(const InputParameters & parameters, bool is_nodal)
  : MooseObject(parameters),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    // VPPs used by ScalarKernels must be broadcast because we don't know where the
    // ScalarKernel will end up being evaluated
    // Note: residual objects should have a valid _moose_base.
    VectorPostprocessorInterface(this,
                                 parameters.get<std::string>("_moose_base") == "ScalarKernel"),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    is_nodal),
    Restartable(this, parameters.get<std::string>("_moose_base") + "s"),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, _sys.number())),
    _mesh(_subproblem.mesh())
{
}

void
ResidualObject::prepareShapes(const unsigned int var_num)
{
  _subproblem.prepareShapes(var_num, _tid);
}

void
ResidualObject::computeResidualAndJacobian()
{
  mooseError(
      "This object has not yet implemented 'computeResidualAndJacobian'. If you would like that "
      "feature for this object, please contact a MOOSE developer.");
}
