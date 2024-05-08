//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyInterfaceDiffusion.h"

registerMooseObject("MooseApp", PenaltyInterfaceDiffusion);
registerMooseObject("MooseApp", ADPenaltyInterfaceDiffusion);
registerMooseObject("MooseApp", VectorPenaltyInterfaceDiffusion);
registerMooseObject("MooseApp", ADVectorPenaltyInterfaceDiffusion);

template <typename T, bool is_ad>
InputParameters
PenaltyInterfaceDiffusionTempl<T, is_ad>::validParams()
{
  InputParameters params = GenericInterfaceKernelTempl<T, is_ad>::validParams();
  params.addRequiredParam<Real>(
      "penalty", "The penalty that penalizes jump between primary and neighbor variables.");
  params.addParam<MaterialPropertyName>(
      "jump_prop_name", "the name of the material property that calculates the jump.");
  params.addClassDescription(
      "A penalty-based interface condition that forces"
      "the continuity of variables and the flux equivalence across an interface.");
  return params;
}

template <typename T, bool is_ad>
PenaltyInterfaceDiffusionTempl<T, is_ad>::PenaltyInterfaceDiffusionTempl(
    const InputParameters & parameters)
  : GenericInterfaceKernelTempl<T, is_ad>(parameters),
    _penalty(this->template getParam<Real>("penalty")),
    _jump(isParamValid("jump_prop_name")
              ? &this->template getGenericMaterialProperty<T, is_ad>("jump_prop_name")
              : nullptr)
{
}

template <typename T, bool is_ad>
GenericReal<is_ad>
PenaltyInterfaceDiffusionTempl<T, is_ad>::computeQpResidual(Moose::DGResidualType type)
{
  GenericReal<is_ad> r = 0;

  Moose::GenericType<T, is_ad> jump_value = 0;

  if (_jump != nullptr)
    jump_value = (*_jump)[_qp];
  else
    jump_value = _u[_qp] - _neighbor_value[_qp];

  switch (type)
  {
    case Moose::Element:
      r = _test[_i][_qp] * _penalty * jump_value;
      break;

    case Moose::Neighbor:
      r = _test_neighbor[_i][_qp] * -_penalty * jump_value;
      break;
  }

  return r;
}

template <typename T, bool is_ad>
Real
PenaltyInterfaceDiffusionTempl<T, is_ad>::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _penalty * _phi[_j][_qp];
      break;

    case Moose::ElementNeighbor:
      jac = _test[_i][_qp] * _penalty * -_phi_neighbor[_j][_qp];
      break;

    case Moose::NeighborElement:
      jac = _test_neighbor[_i][_qp] * -_penalty * _phi[_j][_qp];
      break;

    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * -_penalty * -_phi_neighbor[_j][_qp];
      break;
  }

  return jac;
}

template class PenaltyInterfaceDiffusionTempl<Real, false>;
template class PenaltyInterfaceDiffusionTempl<Real, true>;
template class PenaltyInterfaceDiffusionTempl<RealVectorValue, false>;
template class PenaltyInterfaceDiffusionTempl<RealVectorValue, true>;
