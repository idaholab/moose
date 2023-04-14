//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NeumannBC.h"

registerMooseObject("MooseApp", NeumannBC);
registerMooseObject("MooseApp", ADNeumannBC);

template <bool is_ad>
InputParameters
NeumannBCTempl<is_ad>::validParams()
{
  InputParameters params = GenericIntegratedBC<is_ad>::validParams();
  params.addParam<Real>("value",
                        0.0,
                        "For a Laplacian problem, the value of the gradient dotted with the "
                        "normals on the boundary.");
  params.declareControllable("value");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h$, "
                             "where $h$ is a constant, controllable value.");
  return params;
}

template <bool is_ad>
NeumannBCTempl<is_ad>::NeumannBCTempl(const InputParameters & parameters)
  : GenericIntegratedBC<is_ad>(parameters), _value(this->template getParam<Real>("value"))
{
}

template <bool is_ad>
GenericReal<is_ad>
NeumannBCTempl<is_ad>::computeQpResidual()
{
  return -_test[_i][_qp] * _value;
}

template class NeumannBCTempl<false>;
template class NeumannBCTempl<true>;
