//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearSystemContributionObject.h"
#include "SubProblem.h"
#include "InputParameters.h"
#include "libmesh/linear_implicit_system.h"

InputParameters
LinearSystemContributionObject::validParams()
{
  auto params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += RandomInterface::validParams();
  params += MeshChangedInterface::validParams();
  params += TaggingInterface::validParams();

  MultiMooseEnum vtags("rhs time", "rhs", true);
  auto & vector_tag_enum = params.set<MultiMooseEnum>("vector_tags", true);
  vector_tag_enum = vtags;

  params.addRequiredParam<LinearVariableName>(
      "variable", "The name of the variable whose linear system this object contributes to");

  params.declareControllable("enable");
  params.set<bool>("_residual_object") = false;
  return params;
}

LinearSystemContributionObject::LinearSystemContributionObject(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this, false),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    false),
    Restartable(this, parameters.get<std::string>("_moose_base") + "s"),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(_sys.system())),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _mesh(_subproblem.mesh())
{
}
