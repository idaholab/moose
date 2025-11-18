//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosUserObject.h"

namespace Moose::Kokkos
{

InputParameters
UserObject::validParams()
{
  InputParameters params = ::UserObjectBase::validParams();

  params.registerSystemAttributeName("KokkosUserObject");

  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);

  return params;
}

UserObject::UserObject(const InputParameters & parameters)
  : ::UserObjectBase(parameters),
    MeshHolder(*_fe_problem.mesh().getKokkosMesh()),
    AssemblyHolder(_fe_problem.kokkosAssembly()),
    SystemHolder(_fe_problem.getKokkosSystems())
{
}

UserObject::UserObject(const UserObject & object)
  : ::UserObjectBase(object, {}), MeshHolder(object), AssemblyHolder(object), SystemHolder(object)
{
}

} // namespace Moose::Kokkos
