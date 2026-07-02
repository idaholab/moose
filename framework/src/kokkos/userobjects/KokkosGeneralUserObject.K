//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGeneralUserObject.h"

namespace Moose::Kokkos
{

InputParameters
GeneralUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += ::MaterialPropertyInterface::validParams();
  params += ::TransientInterface::validParams();
  return params;
}

GeneralUserObject::GeneralUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    MeshHolder(*_fe_problem.mesh().getKokkosMesh()),
    AssemblyHolder(_fe_problem.kokkosAssembly()),
    FESystemHolder(_fe_problem.getKokkosFESystems()),
    ::CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    ::MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS),
    ::TransientInterface(this)
{
  // Kokkos user objects currently only consume variable information through finite element bases
  // and quadrature points
  for (auto * const var : getMooseVariableDependencies())
    var->requireQpComputations();
}

GeneralUserObject::GeneralUserObject(const GeneralUserObject & object)
  : UserObject(object),
    MeshHolder(object),
    AssemblyHolder(object),
    FESystemHolder(object),
    ::CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    ::MaterialPropertyInterface(object, {}),
    ::TransientInterface(object, {})
{
}

} // namespace Moose::Kokkos
