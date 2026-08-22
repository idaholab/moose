//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosEFieldAdvection.h"

registerKokkosResidualObject("MooseTestApp", KokkosEFieldAdvection);

InputParameters
KokkosEFieldAdvection::validParams()
{
  InputParameters params = Moose::Kokkos::Kernel::validParams();
  params.addCoupledVar("efield", {1, 1, 1}, "The electric field responsible for charge advection");
  MooseEnum charge("positive negative", "positive");
  params.addParam<MooseEnum>(
      "charge", charge, "Whether our primary variable is positive or negatively charged.");
  params.addParam<Real>("mobility", 1., "The mobility of the charge");
  return params;
}

KokkosEFieldAdvection::KokkosEFieldAdvection(const InputParameters & parameters)
  : Moose::Kokkos::Kernel(parameters),
    _efield_id(coupled("efield")),
    _efield(kokkosCoupledVectorValue("efield")),
    _efield_coupled(isCoupled("efield")),
    _vector_phi(),
    _mobility(getParam<Real>("mobility")),
    _sgn(getParam<MooseEnum>("charge") == "positive" ? 1. : -1.)
{
}
