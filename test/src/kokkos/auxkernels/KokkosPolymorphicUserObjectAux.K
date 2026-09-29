//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosPolymorphicUserObjectAux.h"

registerKokkosAuxKernel("MooseTestApp", KokkosPolymorphicUserObjectAux);

InputParameters
KokkosPolymorphicUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("first", "First polymorphic Kokkos user object.");
  params.addRequiredParam<UserObjectName>("second", "Second polymorphic Kokkos user object.");
  return params;
}

KokkosPolymorphicUserObjectAux::KokkosPolymorphicUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _first(getVirtualKokkosUserObject<KokkosPolymorphicUserObjectBase>("first")),
    _second(getVirtualKokkosUserObject<KokkosPolymorphicUserObjectBase>("second"))
{
}
