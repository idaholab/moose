//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosArrayNeumannBC.h"

registerKokkosResidualObject("MooseApp", KokkosArrayNeumannBC);

InputParameters
KokkosArrayNeumannBC::validParams()
{
  InputParameters params = Moose::Kokkos::ArrayIntegratedBC::validParams();
  params.addParam<RealEigenVector>("value", "The value of the gradient on the boundary.");
  params.declareControllable("value");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h$, "
                             "where $h$ is a constant, controllable value.");
  return params;
}

KokkosArrayNeumannBC::KokkosArrayNeumannBC(const InputParameters & parameters)
  : Moose::Kokkos::ArrayIntegratedBC(parameters),
    _value_host(isParamValid("value") ? getParam<RealEigenVector>("value")
                                      : RealEigenVector::Zero(_count)),
    _value(_value_host)
{
  if (_value_host.size() != _count)
    paramError("value",
               "Number of 'value' entries must equal number of variable components (",
               _count,
               ").");
}
