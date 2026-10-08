//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorNeumannBC.h"

registerKokkosResidualObject("MooseApp", KokkosVectorNeumannBC);

InputParameters
KokkosVectorNeumannBC::validParams()
{
  InputParameters params = Moose::Kokkos::VectorIntegratedBCValue::validParams();
  params.addParam<RealVectorValue>(
      "values",
      RealVectorValue(0, 0, 0),
      "The values of the gradient components dotted with the normals on the boundary.");
  params.declareControllable("values");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial \\vec{u}}{\\partial n}=\\vec{h}$, where $\\vec{h}$ "
                             "are constant, controllable values.");
  return params;
}

KokkosVectorNeumannBC::KokkosVectorNeumannBC(const InputParameters & parameters)
  : Moose::Kokkos::VectorIntegratedBCValue(parameters),
    _values_host(getParam<RealVectorValue>("values")),
    _values(_values_host)
{
}

KokkosVectorNeumannBC::KokkosVectorNeumannBC(const KokkosVectorNeumannBC & object)
  : Moose::Kokkos::VectorIntegratedBCValue(object),
    _values_host(object._values_host),
    _values(_values_host)
{
}
