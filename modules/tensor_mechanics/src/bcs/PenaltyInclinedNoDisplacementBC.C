//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyInclinedNoDisplacementBC.h"
#include "Function.h"
#include "Coupleable.h"

registerMooseObject("TensorMechanicsApp", PenaltyInclinedNoDisplacementBC);
registerMooseObject("TensorMechanicsApp", ADPenaltyInclinedNoDisplacementBC);

template <bool is_ad>
InputParameters
PenaltyInclinedNoDisplacementBCTempl<is_ad>::validParams()
{
  InputParameters params = PenaltyInclinedNoDisplacementBCParent<is_ad>::validParams();
  params.addRequiredParam<Real>("penalty", "Penalty parameter");
  params.addRequiredParam<unsigned int>(
      "component", "An integer corresponding to the direction (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addClassDescription("Penalty Enforcement of an inclined boundary condition");
  return params;
}

template <bool is_ad>
PenaltyInclinedNoDisplacementBCTempl<is_ad>::PenaltyInclinedNoDisplacementBCTempl(
    const InputParameters & parameters)
  : PenaltyInclinedNoDisplacementBCParent<is_ad>(parameters),
    _component(this->template getParam<unsigned int>("component")),
    _ndisp(this->coupledComponents("displacements")),
    _disp(this->template coupledGenericValues<is_ad>("displacements")),
    _disp_var(this->coupledIndices("displacements")),
    _penalty(this->template getParam<Real>("penalty"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PenaltyInclinedNoDisplacementBCTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> v = 0;
  for (unsigned int i = 0; i < _ndisp; ++i)
    v += (*_disp[i])[_qp] * _normals[_qp](i);

  return _penalty * _test[_i][_qp] * v * _normals[_qp](_component);
}

Real
PenaltyInclinedNoDisplacementBC::computeQpJacobian()
{
  return _penalty * _phi[_j][_qp] * _normals[_qp](_component) * _normals[_qp](_component) *
         _test[_i][_qp];
}

Real
PenaltyInclinedNoDisplacementBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int coupled_component = 0; coupled_component < _ndisp; ++coupled_component)
    if (jvar == _disp_var[coupled_component])
    {
      return _penalty * _phi[_j][_qp] * _normals[_qp](coupled_component) *
             _normals[_qp](_component) * _test[_i][_qp];
    }

  return 0.0;
}

template class PenaltyInclinedNoDisplacementBCTempl<false>;
template class PenaltyInclinedNoDisplacementBCTempl<true>;
