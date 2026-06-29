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
    Restartable(this, getBase() + "s"),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    MeshHolder(*getCheckedPointerParam<SubProblem *>("_subproblem")->mesh().getKokkosMesh()),
    SystemHolder(getCheckedPointerParam<SystemBase *>("_sys")->feProblem().getKokkosSystems()),
    _fe_problem(*getParam<FEProblemBase *>("_fe_problem_base")),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  const auto * const subproblem = getCheckedPointerParam<SubProblem *>("_subproblem");
  _kokkos_var.init(subproblem->getVariable(0,
                                           getParam<LinearVariableName>("variable"),
                                           Moose::VarKindType::VAR_SOLVER,
                                           Moose::VarFieldType::VAR_FIELD_STANDARD),
                   Moose::SOLUTION_TAG);
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
    Restartable(object, {}),
    MeshChangedInterface(object, {}),
    TaggingInterface(object, {}),
    MeshHolder(object),
    SystemHolder(object),
    _fe_problem(object._fe_problem),
    _kokkos_var(object._kokkos_var),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old)
{
  // Tags may be added to the governing tag sets held by the TaggingInterface later than the time
  // that the primary constructor is run. So it's more robust to build our tag set, immediately
  // before being dispatched to device, from the governing TaggingInterface set instead of relying
  // on copying the tag set from object's \p _vector_tags and \p _matrix_tags arrays
  _vector_tags = object.getVectorTags({});
  _matrix_tags = object.getMatrixTags({});
}

} // namespace Moose::Kokkos
