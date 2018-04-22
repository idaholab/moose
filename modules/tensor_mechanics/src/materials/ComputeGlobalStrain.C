//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeGlobalStrain.h"
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeGlobalStrain);

template <>
InputParameters
validParams<ComputeGlobalStrain>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Material for storing the global strain values from the scalar variable");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addCoupledVar("scalar_global_strain", "Scalar variable for global strain");
  return params;
}

ComputeGlobalStrain::ComputeGlobalStrain(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _scalar_global_strain(coupledScalarValue("scalar_global_strain")),
    _scalar_global_strain_order(coupledScalarOrder("scalar_global_strain")),
    _global_strain(declareProperty<RankTwoTensor>(_base_name + "global_strain"))
{
}

void
ComputeGlobalStrain::initQpStatefulProperties()
{
  _global_strain[_qp].zero();
}

void
ComputeGlobalStrain::computeProperties()
{
  RankTwoTensor & strain = _global_strain[0];
  switch (_scalar_global_strain_order)
  {
    case 1:
      strain = RankTwoTensor(_scalar_global_strain[0], 0, 0, 0, 0, 0);
      break;

    case 3:
      strain = RankTwoTensor(
          _scalar_global_strain[0], _scalar_global_strain[1], 0, 0, 0, _scalar_global_strain[2]);
      break;

    case 6:
      strain = RankTwoTensor(_scalar_global_strain[0],
                             _scalar_global_strain[1],
                             _scalar_global_strain[2],
                             _scalar_global_strain[3],
                             _scalar_global_strain[4],
                             _scalar_global_strain[5]);
      break;

    default:
      mooseError("ComputePerdiodicStrain is only compatible with FIRST, THIRD, and SIXTH "
                 "order scalar variables.");
  }

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    _global_strain[_qp] = strain;
}
