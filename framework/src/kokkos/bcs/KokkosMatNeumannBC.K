//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMatNeumannBC.h"

registerKokkosResidualObject("MooseApp", KokkosMatNeumannBC);

InputParameters
KokkosMatNeumannBC::validParams()
{
  InputParameters params = KokkosNeumannBC::validParams();
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{C \\partial u}{\\partial n}=M*h$, "
                             "where $h$ is a constant, $M$ is a material property, and $C$ is a "
                             "coefficient defined by the kernel for $u$.");
  params.addRequiredParam<MaterialPropertyName>(
      "boundary_material",
      "Material property multiplying the constant that will be enforced by the BC");
  return params;
}

KokkosMatNeumannBC::KokkosMatNeumannBC(const InputParameters & parameters)
  : KokkosNeumannBC(parameters),
    _boundary_prop(getKokkosMaterialProperty<Real>("boundary_material"))
{
}
