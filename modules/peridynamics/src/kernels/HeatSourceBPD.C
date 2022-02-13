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

InputParameters
HeatSourceBPD::validParams()
{
  InputParameters params = PeridynamicsKernelBase::validParams();
  params.addClassDescription(
      "Class for calculating the residual from heat source for the bond-based "
      "peridynamic heat conduction formulation");

  params.addRequiredParam<FunctionName>("power_density", "Volumetric heat source density");

  return params;
}

HeatSourceBPD::HeatSourceBPD(const InputParameters & parameters)
  : PeridynamicsKernelBase(parameters), _power_density(getFunction("power_density"))
{
}

void
HeatSourceBPD::computeLocalResidual()
{
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    _local_re(nd) = -_power_density.value(_t) * _node_vol[nd] / neighbors.size();
  }
}
