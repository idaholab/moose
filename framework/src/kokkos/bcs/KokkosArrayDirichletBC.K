//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosArrayDirichletBC.h"

registerKokkosResidualObject("MooseApp", KokkosArrayDirichletBC);

InputParameters
KokkosArrayDirichletBC::validParams()
{
  InputParameters params = Moose::Kokkos::ArrayNodalBC::validParams();
  params.addRequiredParam<RealEigenVector>("values",
                                           "The values the components must take on the boundary");
  params.declareControllable("values");
  params.addClassDescription(
      "Imposes the essential boundary condition $\\vec{u}=\\vec{g}$, where $\\vec{g}$ "
      "are constant, controllable values.");
  return params;
}

KokkosArrayDirichletBC::KokkosArrayDirichletBC(const InputParameters & parameters)
  : Moose::Kokkos::ArrayNodalBC(parameters), _values(getParam<RealEigenVector>("values"))
{
  if (getParam<RealEigenVector>("values").size() != _count)
    paramError(
        "values", "Number of 'values' must equal number of variable components (", _count, ").");
}
