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
    _var(_subproblem.getVariable(_tid,
                                 getParam<NonlinearVariableName>("variable"),
                                 Moose::VarKindType::VAR_SOLVER,
                                 field_type)),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  _kokkos_var.init(_var);
}

ResidualObject::ResidualObject(const ResidualObject & object)
  : ::ResidualObject(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    SystemHolder(object),
    _var(object._var),
    _kokkos_var(object._kokkos_var),
    _thread(object._thread),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old)
{
  _vector_tags = object.getVectorTags({});
  _matrix_tags = object.getMatrixTags({});
}

} // namespace Moose::Kokkos
