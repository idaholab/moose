//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedGapAux.h"

registerMooseObject("MooseApp", WeightedGapAux);

InputParameters
WeightedGapAux::validParams()
{
  InputParameters params = MortarNodalAuxKernel::validParams();
  params.addClassDescription(
      "Returns the specified variable as an auxiliary variable with the same value.");
  params.set<bool>("interpolate_normals") = false;
  return params;
}

WeightedGapAux::WeightedGapAux(const InputParameters & parameters)
  : MortarNodalAuxKernel(parameters), _weighted_gap(0), _qp_gap_nodal(0)
{
}

Real
WeightedGapAux::computeValue()
{
  _weighted_gap = 0;
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test_lower.size(); ++_i)
      computeQpIProperties();
  }

  return _weighted_gap;
}

void
WeightedGapAux::computeQpProperties()
{
  const auto gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord_msm[_qp]);

  _msm_volume += _JxW_msm[_qp] * _coord_msm[_qp];
}

void
WeightedGapAux::computeQpIProperties()
{
  _weighted_gap += _test_lower[_i][_qp] * _qp_gap_nodal * _normals[_i];
}
