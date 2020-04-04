//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowThermalConductivityBase.h"

InputParameters
PorousFlowThermalConductivityBase::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.set<bool>("at_nodes") = false;
  params.set<std::string>("pf_material_type") = "thermal_conductivity";
  params.addClassDescription("Base class Material for thermal conductivity");
  return params;
}

PorousFlowThermalConductivityBase::PorousFlowThermalConductivityBase(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _la_qp(declareProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp")),
    _dla_qp_dvar(
        declareProperty<std::vector<RealTensorValue>>("dPorousFlow_thermal_conductivity_qp_dvar"))
{
  if (_nodal_material == true)
    mooseError("PorousFlowThermalConductivity classes are only defined for at_nodes = false");
}
