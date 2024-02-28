//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThermalExpansionEigenstrainBeamBase.h"

InputParameters
ComputeThermalExpansionEigenstrainBeamBase::validParams()
{
  InputParameters params = ComputeEigenstrainBeamBase::validParams();
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addRequiredCoupledVar("stress_free_temperature",
                               "Reference temperature at which there is no "
                               "thermal expansion for thermal eigenstrain "
                               "calculation");
  return params;
}

ComputeThermalExpansionEigenstrainBeamBase::ComputeThermalExpansionEigenstrainBeamBase(
    const InputParameters & parameters)
  : ComputeEigenstrainBeamBase(parameters),
    _temperature(coupledValue("temperature")),
    _stress_free_temperature(coupledValue("stress_free_temperature"))
{
}

void
ComputeThermalExpansionEigenstrainBeamBase::computeQpEigenstrain()
{
  // fetch the two end nodes for current element
  std::vector<const Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->node_ptr(i));

  // calculate initial axis of the beam element
  for (unsigned int i = 0; i < 2; ++i)
    _initial_axis(i) = (*node[1])(i) - (*node[0])(i);

  _initial_axis /= _initial_axis.norm();

  Real thermal_strain = computeThermalStrain();

  _disp_eigenstrain[_qp].zero();
  _rot_eigenstrain[_qp].zero();
  _disp_eigenstrain[_qp] = _initial_axis * thermal_strain;
}
