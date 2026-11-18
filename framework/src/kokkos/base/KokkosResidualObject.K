//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosResidualObject.h"

#include "FEProblemBase.h"

namespace Moose::Kokkos
{

InputParameters
ResidualObject::validParams()
{
  auto params = ::ResidualObject::validParams();

  params.makeParamNotRequired<NonlinearVariableName>("variable");
  params.addParam<std::vector<NonlinearVariableName>>(
      "variables", "The names of the variables that this residual object operates on");

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");

  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addCoupledVar("displacements", "The displacements");

  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);

  return params;
}

ResidualObject::ResidualObject(const InputParameters & parameters,
                               Moose::VarFieldType field_type,
                               bool nodal)
  : ::ResidualObject(parameters, nodal),
    MeshHolder(*_fe_problem.mesh().getKokkosMesh()),
    AssemblyHolder(_fe_problem.kokkosAssembly()),
    FESystemHolder(_fe_problem.getKokkosFESystems()),
    _vars(getMooseVariables(field_type)),
    _dimension(_fe_problem.mesh().dimension()),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  _kokkos_var.init(_vars);
}

std::vector<MooseVariableFieldBase *>
ResidualObject::getMooseVariables(Moose::VarFieldType field_type)
{
  const auto var_param_name = parameters().varParamName();

  std::vector<NonlinearVariableName> names;

  if (var_param_name == "variable")
    names.push_back(getParam<NonlinearVariableName>("variable"));
  else
    names = getParam<std::vector<NonlinearVariableName>>("variables");

  std::vector<MooseVariableFieldBase *> vars;

  for (const auto & name : names)
    vars.push_back(
        &_subproblem.getVariable(_tid, name, Moose::VarKindType::VAR_SOLVER, field_type));

  for (unsigned int i = 1; i < vars.size(); ++i)
  {
    if (vars[0]->sys().number() != vars[i]->sys().number())
      mooseError("For multi-variable kernels/BCs, all variables should reside in the same system.");

    if (vars[0]->feType() != vars[i]->feType())
      mooseError("For multi-variable kernels/BCs, all variables should have the same FE family "
                 "and order.");
  }

  for (const auto var : vars)
    if (var->isVector() && field_type != Moose::VarFieldType::VAR_FIELD_VECTOR)
      paramError(var_param_name, "Vector variable specified to a scalar Kokkos residual object.");

  return vars;
}

ResidualObject::ResidualObject(const ResidualObject & object)
  : ::ResidualObject(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    FESystemHolder(object),
    _vars(object._vars),
    _kokkos_var(object._kokkos_var),
    _thread(object._thread),
    _dimension(object._dimension),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old),
    _vector_tags(object._vector_tags),
    _matrix_tags(object._matrix_tags)
{
  // Tags may be added to the governing tag sets held by the TaggingInterface later than the time
  // that the primary constructor is run. So it's more robust to build our tag set, immediately
  // before being dispatched to device, from the governing TaggingInterface set instead of relying
  // on copying the tag set from object's \p _vector_tags and \p _matrix_tags arrays
  _vector_tags = object.getVectorTags({});
  _matrix_tags = object.getMatrixTags({});
}

} // namespace Moose::Kokkos
