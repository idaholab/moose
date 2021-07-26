//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatNeumannBC.h"

registerMooseObject("MooseApp", MatNeumannBC);
registerMooseObject("MooseApp", ADMatNeumannBC);

template <bool is_ad>
InputParameters
MatNeumannBCTempl<is_ad>::validParams()
{
  InputParameters params = NeumannBCTempl<is_ad>::validParams();

  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{C \\partial u}{\\partial n}=M*h$, "
                             "where $h$ is a constant, $M$ is a material property, and $C$ is a "
                             "coefficient defined by the kernel for $u$.");
  params.addRequiredParam<MaterialPropertyName>(
      "boundary_material",
      "Material property multiplying the constant that will be enforced by the BC");
  return params;
}

template <bool is_ad>
MatNeumannBCTempl<is_ad>::MatNeumannBCTempl(const InputParameters & parameters)
  : NeumannBCTempl<is_ad>(parameters),
    _boundary_prop(this->template getGenericMaterialProperty<Real, is_ad>("boundary_material"))
{
}

template <bool is_ad>
GenericReal<is_ad>
MatNeumannBCTempl<is_ad>::computeQpResidual()
{
  return -_test[_i][_qp] * _value * _boundary_prop[_qp];
}
