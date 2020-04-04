//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowTotalGravitationalDensityBase.h"

InputParameters
PorousFlowTotalGravitationalDensityBase::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.set<std::string>("pf_material_type") = "gravitational_density";
  params.addClassDescription("Base class Material for porous medium density");
  return params;
}

PorousFlowTotalGravitationalDensityBase::PorousFlowTotalGravitationalDensityBase(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _gravdensity(declareProperty<Real>("density")),
    _dgravdensity_dvar(declareProperty<std::vector<Real>>("ddensity_dvar"))
{
}
