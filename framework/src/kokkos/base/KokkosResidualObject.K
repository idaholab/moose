//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
    SystemHolder(_fe_problem.getKokkosSystems()),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  if (isParamValid("variable") && isParamValid("variables"))
    mooseError("Cannot specify both 'variable' and 'variables' at the same time.");
  else if (!isParamValid("variable") && !isParamValid("variables"))
    mooseError("Either 'variable' or 'variables' should be specified.");

  std::vector<NonlinearVariableName> variables;

  if (isParamValid("variable"))
    variables.push_back(getParam<NonlinearVariableName>("variable"));
  else
    variables = getParam<std::vector<NonlinearVariableName>>("variables");

  for (const auto & variable : variables)
    _vars.push_back(
        &_subproblem.getVariable(_tid, variable, Moose::VarKindType::VAR_SOLVER, field_type));

  if (_vars.size() > 1)
    for (unsigned int i = 1; i < _vars.size(); ++i)
    {
      if (_vars[0]->sys().number() != _vars[i]->sys().number())
        mooseError(
            "For multi-variable kernels/BCs, all variables should reside in the same system.");

      if (_vars[0]->feType() != _vars[i]->feType())
        mooseError("For multi-variable kernels/BCs, all variables should have the same FE family "
                   "and order.");
    }

  _kokkos_var.init(_vars);
}

ResidualObject::ResidualObject(const ResidualObject & object)
  : ::ResidualObject(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    SystemHolder(object),
    _kokkos_var(object._kokkos_var),
    _thread(object._thread),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old),
    _vars(object._vars)
{
  _vector_tags = object.getVectorTags({});
  _matrix_tags = object.getMatrixTags({});
}

} // namespace Moose::Kokkos
