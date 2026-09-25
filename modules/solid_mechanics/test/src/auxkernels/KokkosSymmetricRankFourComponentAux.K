//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSymmetricRankFourComponentAux.h"

registerKokkosAuxKernel("SolidMechanicsTestApp", KokkosSymmetricRankFourComponentAux);

InputParameters
KokkosSymmetricRankFourComponentAux::validParams()
{
  InputParameters params = Moose::Kokkos::AuxKernel::validParams();
  params.addClassDescription("Write one stored Mandel component of a symmetric fourth-order Kokkos "
                             "material property into an auxiliary variable.");
  params.addRequiredParam<MaterialPropertyName>("property", "The property to read");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "i", "i < 6", "Stored Mandel row index, in the range [0, 6)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "j", "j < 6", "Stored Mandel column index, in the range [0, 6)");
  return params;
}

KokkosSymmetricRankFourComponentAux::KokkosSymmetricRankFourComponentAux(
    const InputParameters & parameters)
  : Moose::Kokkos::AuxKernel(parameters),
    _prop(getKokkosMaterialProperty<Moose::Kokkos::Real66>("property")),
    _i(getParam<unsigned int>("i")),
    _j(getParam<unsigned int>("j"))
{
}
