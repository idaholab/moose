//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatFluxFromHeatStructureBaseUserObject.h"

InputParameters
ADHeatFluxFromHeatStructureBaseUserObject::validParams()
{
  InputParameters params = FlowChannelHeatStructureCouplerUserObject::validParams();
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  params.addClassDescription(
      "Base class for caching heat flux between flow channels and heat structures.");
  return params;
}

ADHeatFluxFromHeatStructureBaseUserObject::ADHeatFluxFromHeatStructureBaseUserObject(
    const InputParameters & parameters)
  : FlowChannelHeatStructureCouplerUserObject(parameters), _qp(0), _P_hf(adCoupledValue("P_hf"))
{
}

const std::vector<ADReal> &
ADHeatFluxFromHeatStructureBaseUserObject::getHeatedPerimeter(dof_id_type element_id) const
{
  return getCachedQuantity(element_id, _heated_perimeter, "heated perimeter");
}

const std::vector<ADReal> &
ADHeatFluxFromHeatStructureBaseUserObject::getHeatFlux(dof_id_type element_id) const
{
  return getCachedQuantity(element_id, _heat_flux, "heat flux");
}

std::vector<std::map<dof_id_type, std::vector<ADReal>> *>
ADHeatFluxFromHeatStructureBaseUserObject::getCachedQuantityMaps()
{
  return {&_heated_perimeter, &_heat_flux};
}

std::vector<const std::map<dof_id_type, std::vector<ADReal>> *>
ADHeatFluxFromHeatStructureBaseUserObject::getCachedQuantityMaps() const
{
  return {&_heated_perimeter, &_heat_flux};
}

void
ADHeatFluxFromHeatStructureBaseUserObject::computeQpCachedQuantities()
{
  _qp = _fc_qp;
  const ADReal q_wall = computeQpHeatFlux();

  _heat_flux[_fc_elem_id][_fc_qp] = q_wall;
  _heat_flux[_hs_elem_id][_hs_qp] = q_wall;

  _heated_perimeter[_fc_elem_id][_fc_qp] = _P_hf[_fc_qp];
  _heated_perimeter[_hs_elem_id][_hs_qp] = _P_hf[_fc_qp];
}
