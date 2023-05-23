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

registerMooseObject("TensorMechanicsApp", PenaltyInclinedNoDisplacementBC);

InputParameters
PenaltyInclinedNoDisplacementBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<Real>("penalty", "Penalty parameter");
  params.addRequiredParam<unsigned int>(
      "component", "An integer corresponding to the direction (0 for x, 1 for y, 2 for z)");
  params.addParam<bool>(
      "normalize_penalty",
      false,
      "Whether to normalize the penalty factor of this constraint with the element side area.");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addClassDescription("Penalty enforcement of an inclined boundary condition");

  return params;
}

PenaltyInclinedNoDisplacementBC::PenaltyInclinedNoDisplacementBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _disp_var(coupledIndices("displacements")),
    _penalty(getParam<Real>("penalty")),
    _normalize_penalty(getParam<bool>("normalize_penalty"))
{
}

Real
PenaltyInclinedNoDisplacementBC::computeQpResidual()
{
  Real v = 0;
  for (unsigned int i = 0; i < _ndisp; ++i)
    v += (*_disp[i])[_qp] * _normals[_qp](i);

  auto penalty = _penalty;

  if (_normalize_penalty)
    penalty /= _current_side_volume;

  return penalty * _test[_i][_qp] * v * _normals[_qp](_component);
}

Real
PenaltyInclinedNoDisplacementBC::computeQpJacobian()
{
  auto penalty = _penalty;

  if (_normalize_penalty)
    penalty /= _current_side_volume;

  return penalty * _phi[_j][_qp] * _normals[_qp](_component) * _normals[_qp](_component) *
         _test[_i][_qp];
}

Real
PenaltyInclinedNoDisplacementBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  auto penalty = _penalty;

  if (_normalize_penalty)
    penalty /= _current_side_volume;

  for (unsigned int coupled_component = 0; coupled_component < _ndisp; ++coupled_component)
    if (jvar == _disp_var[coupled_component])
    {
      return penalty * _phi[_j][_qp] * _normals[_qp](coupled_component) *
             _normals[_qp](_component) * _test[_i][_qp];
    }

  return 0.0;
}
