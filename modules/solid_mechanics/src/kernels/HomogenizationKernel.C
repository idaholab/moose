/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HomogenizationKernel.h"
#include "Material.h"
#include "SymmElasticityTensor.h"

template <>
InputParameters
validParams<HomogenizationKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredRangeCheckedParam<unsigned int>("component",
                                                    "component >= 0 & component < 3",
                                                    "An integer corresponding to the direction "
                                                    "the variable this kernel acts in. (0 for x, "
                                                    "1 for y, 2 for z)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "column",
      "column >= 0 & column < 6",
      "An integer corresponding to the direction the "
      "variable this kernel acts in. (0 for xx, 1 for yy, 2 "
      "for zz, 3 for xy, 4 for yz, 5 for zx)");
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");

  return params;
}

HomogenizationKernel::HomogenizationKernel(const InputParameters & parameters)
  : Kernel(parameters),

    _elasticity_tensor(getMaterialProperty<SymmElasticityTensor>(
        "elasticity_tensor" + getParam<std::string>("appended_property_name"))),
    _component(getParam<unsigned int>("component")),
    _column(getParam<unsigned int>("column"))
{
}

Real
HomogenizationKernel::computeQpResidual()
{
  unsigned k = 0, l = 0;

  if (_column == 0)
  {
    k = 0;
    l = 0;
  }

  if (_column == 1)
  {
    k = 1;
    l = 1;
  }

  if (_column == 2)
  {
    k = 2;
    l = 2;
  }

  if (_column == 3)
  {
    k = 0;
    l = 1;
  }

  if (_column == 4)
  {
    k = 1;
    l = 2;
  }

  if (_column == 5)
  {
    k = 2;
    l = 0;
  }

  const unsigned J(3 * l + k);

  ColumnMajorMatrix E(_elasticity_tensor[_qp].columnMajorMatrix9x9());

  Real value(0);

  // Compute positive value since we are computing a residual not a rhs
  for (unsigned j = 0; j < 3; j++)
  {
    const unsigned I = 3 * j + _component;
    value += E(I, J) * _grad_test[_i][_qp](j);
  }

  return value;
}
