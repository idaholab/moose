//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AsymptoticExpansionHomogenizationKernel.h"

registerMooseObject("TensorMechanicsApp", AsymptoticExpansionHomogenizationKernel);

InputParameters
AsymptoticExpansionHomogenizationKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Kernel for asymptotic expansion homogenization for elasticity");
  params.addRequiredRangeCheckedParam<unsigned int>("component",
                                                    "component >= 0 & component < 3",
                                                    "An integer corresponding to the direction "
                                                    "the variable this kernel acts in. (0 for x, "
                                                    "1 for y, 2 for z)");
  MooseEnum column("xx yy zz yz xz xy");
  params.addRequiredParam<MooseEnum>("column",
                                     column,
                                     "The column of the material matrix this kernel acts in. "
                                     "(xx, yy, zz, yz, xz, or xy)");

  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");

  return params;
}

AsymptoticExpansionHomogenizationKernel::AsymptoticExpansionHomogenizationKernel(
    const InputParameters & parameters)
  : Kernel(parameters),

    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),
    _component(getParam<unsigned int>("component")),
    _column(getParam<MooseEnum>("column")),
    _k_index({{0, 1, 2, 1, 0, 0}}),
    _l_index({{0, 1, 2, 2, 2, 1}}),
    _k(_k_index[_column]),
    _l(_l_index[_column])
{
}

Real
AsymptoticExpansionHomogenizationKernel::computeQpResidual()
{
  Real value = 0;

  for (unsigned j = 0; j < 3; j++)
    value += _grad_test[_i][_qp](j) * _elasticity_tensor[_qp](_component, j, _k, _l);

  return value;
}
