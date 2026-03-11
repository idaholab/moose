//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearSystemContributionObject.h"

#include "FEProblemBase.h"
#include "InputParameters.h"

namespace Moose::Kokkos
{

InputParameters
LinearSystemContributionObject::validParams()
{
  auto params = ::LinearSystemContributionObject::validParams();
  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);
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
    Restartable(this, getBase() + "s"),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    MeshHolder(
        *parameters.getCheckedPointerParam<SubProblem *>("_subproblem")->mesh().getKokkosMesh()),
    AssemblyHolder(
        parameters.getCheckedPointerParam<SystemBase *>("_sys")->feProblem().kokkosAssembly()),
    SystemHolder(
        parameters.getCheckedPointerParam<SystemBase *>("_sys")->feProblem().getKokkosSystems()),
    _subproblem(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*parameters.getCheckedPointerParam<SystemBase *>("_sys")),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(_sys.system())),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _mesh(_subproblem.mesh()),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  _vector_tags = getVectorTags({});
  _matrix_tags = getMatrixTags({});
}

LinearSystemContributionObject::LinearSystemContributionObject(
    const LinearSystemContributionObject & object)
  : MooseObject(object, {}),
    SetupInterface(object, {}),
    FunctionInterface(object, {}),
    UserObjectInterface(object, {}),
    TransientInterface(object, {}),
    PostprocessorInterface(object, {}),
    VectorPostprocessorInterface(object, {}),
    RandomInterface(object, {}),
    Restartable(object, {}),
    MeshChangedInterface(object, {}),
    TaggingInterface(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    SystemHolder(object),
    _subproblem(object._subproblem),
    _fe_problem(object._fe_problem),
    _sys(object._sys),
    _linear_system(object._linear_system),
    _tid(object._tid),
    _mesh(object._mesh),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old)
{
  _vector_tags = object._vector_tags;
  _matrix_tags = object._matrix_tags;
}

} // namespace Moose::Kokkos
