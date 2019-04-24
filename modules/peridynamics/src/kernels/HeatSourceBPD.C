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
  params.addParam<Real>("power_density", 0, "Volumetric heat source density");
  params.addParam<FunctionName>("power_density_function",
                                "Function describing the volumetric heat source density");

  return params;
}

HeatSourceBPD::HeatSourceBPD(const InputParameters & parameters)
  : KernelBasePD(parameters),
    _power_density(getParam<Real>("power_density")),
    _power_density_function(
        isParamValid("power_density_function") ? &getFunction("power_density_function") : NULL)
{
}

void
HeatSourceBPD::computeLocalResidual()
{
  // get the total_bonds for each node
  unsigned int nn_i = _pdmesh.getNNeighbors(_current_elem->node_id(0));
  unsigned int nn_j = _pdmesh.getNNeighbors(_current_elem->node_id(1));

  if (_power_density_function)
  {
    Point p;
    _power_density = _power_density_function->value(_t, p);
  }

  _local_re(0) = -_power_density * _vols_ij[0] / nn_i;
  _local_re(1) = -_power_density * _vols_ij[1] / nn_j;
}
