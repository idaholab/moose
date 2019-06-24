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
  InputParameters params = validParams<PeridynamicsKernelBase>();
  params.addClassDescription("Class for calculating residual from heat source for bond-based "
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
  Point p;
  for (unsigned int i = 0; i < _nnodes; ++i)
  {
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(i));
    _local_re(i) = _power_density.value(_t, p) * _vols_ij[i] / neighbors.size();
  }
}
