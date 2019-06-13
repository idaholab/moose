//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatSourceBPD.h"
#include "PeridynamicsMesh.h"
#include "Function.h"

registerMooseObject("PeridynamicsApp", HeatSourceBPD);

template <>
InputParameters
validParams<HeatSourceBPD>()
{
  InputParameters params = validParams<KernelBasePD>();
  params.addClassDescription("Class for calculating residual from heat source for bond-based "
                             "peridynamic heat conduction formulation");
  params.addRequiredParam<FunctionName>("power_density", "Volumetric heat source density");

  return params;
}

HeatSourceBPD::HeatSourceBPD(const InputParameters & parameters)
  : KernelBasePD(parameters), _power_density(getFunction("power_density"))
{
}

void
HeatSourceBPD::computeLocalResidual()
{
  // get the total_bonds for each node
  unsigned int nn_i = _pdmesh.getNNeighbors(_current_elem->node_id(0));
  unsigned int nn_j = _pdmesh.getNNeighbors(_current_elem->node_id(1));
  Point p;

  _local_re(0) = -_power_density.value(_t, p) * _vols_ij[0] / nn_i;
  _local_re(1) = -_power_density.value(_t, p) * _vols_ij[1] / nn_j;
}
