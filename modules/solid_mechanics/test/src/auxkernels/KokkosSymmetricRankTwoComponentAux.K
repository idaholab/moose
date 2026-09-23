//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSymmetricRankTwoComponentAux.h"

registerKokkosAuxKernel("SolidMechanicsTestApp", KokkosSymmetricRankTwoComponentAux);

InputParameters
KokkosSymmetricRankTwoComponentAux::validParams()
{
  InputParameters params = Moose::Kokkos::AuxKernel::validParams();
  params.addClassDescription("Write one component of a Mandel symmetric second-order Kokkos "
                             "material property into an auxiliary variable.");
  params.addRequiredParam<MaterialPropertyName>("property", "The property to read");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component", "component < 6", "The stored Mandel component index");
  return params;
}

KokkosSymmetricRankTwoComponentAux::KokkosSymmetricRankTwoComponentAux(
    const InputParameters & parameters)
  : Moose::Kokkos::AuxKernel(parameters),
    _prop(getKokkosMaterialProperty<Moose::Kokkos::Real6>("property")),
    _component(getParam<unsigned int>("component"))
{
}
