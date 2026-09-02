//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSCZMInterfaceKernelSmallStrain.h"
#include "ElasticityTensorTools.h"

registerMooseObject("ShiftedBoundaryMethodApp", ADSCZMInterfaceKernelSmallStrain);

InputParameters
ADSCZMInterfaceKernelSmallStrain::validParams()
{
  InputParameters params = ADSCZMInterfaceKernelBase::validParams();

  params.addParam<bool>("directional_correction", true, "Add the directional correction terms.");
  params.addParam<MaterialPropertyName>(
      "stress", "stress", "Name of the stress tensor material property.");
  params.addClassDescription(
      "Shifted CZM Interface kernel to use when using the Small Strain kinematic formulation.");

  return params;
}

ADSCZMInterfaceKernelSmallStrain::ADSCZMInterfaceKernelSmallStrain(
    const InputParameters & parameters)
  : ADSCZMInterfaceKernelBase(parameters),
    _stress(getADMaterialPropertyByName<RankTwoTensor>(
        _base_name + getParam<MaterialPropertyName>("stress"))),
    _stress_neighbor(getNeighborADMaterialPropertyByName<RankTwoTensor>(
        _base_name + getParam<MaterialPropertyName>("stress"))),
    _directional_correction(getParam<bool>("directional_correction"))
{
}

ADReal
ADSCZMInterfaceKernelSmallStrain::computeQpResidual(Moose::DGResidualType type)
{
  if (!_shifted)
    return ADSCZMInterfaceKernelBase::computeQpResidual(type);

  const auto true_normal = RealVectorValue(trueNormal());
  const auto true_normal_dot_surrogate_normal = true_normal * _normals[_qp];

  auto residual = ADSCZMInterfaceKernelBase::computeQpResidual(type);
  residual *= true_normal_dot_surrogate_normal;

  if (_directional_correction)
  {
    const auto stress = _stress[_qp].row(_component);
    const auto stress_neigh = _stress_neighbor[_qp].row(_component);

    const auto nt_tagent = _normals[_qp] - true_normal_dot_surrogate_normal * true_normal;

    switch (type)
    {
      case Moose::Element:
        residual -= _test[_i][_qp] * stress * nt_tagent;
        break;

      case Moose::Neighbor:
        residual += _test_neighbor[_i][_qp] * stress_neigh * nt_tagent;
        break;

      default:
        break;
    }
  }

  return residual;
}
