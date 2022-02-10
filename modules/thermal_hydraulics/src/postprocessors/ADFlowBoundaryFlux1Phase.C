//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFlowBoundaryFlux1Phase.h"
#include "ADBoundaryFluxBase.h"
#include "THMIndices3Eqn.h"

registerMooseObject("ThermalHydraulicsApp", ADFlowBoundaryFlux1Phase);

InputParameters
ADFlowBoundaryFlux1Phase::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  MooseEnum equation("mass=0 momentum=1 energy=2");
  params.addRequiredParam<MooseEnum>(
      "equation", equation, "Equation for which to query flux vector");
  params.addCoupledVar("variables", "Single-phase flow variables");
  params.set<std::vector<VariableName>>("variables") = {"rhoA", "rhouA", "rhoEA", "A"};
  params.addClassDescription(
      "Retrieves an entry of a flux vector for a connection attached to a 1-phase junction");

  return params;
}

ADFlowBoundaryFlux1Phase::ADFlowBoundaryFlux1Phase(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _n_components(THM3Eqn::N_CONS_VAR),
    _boundary_name(getParam<std::vector<BoundaryName>>("boundary")[0]),
    _boundary_uo_name(_boundary_name + ":boundary_uo"),
    _boundary_uo(getUserObjectByName<ADBoundaryFluxBase>(_boundary_uo_name)),
    _equation_index(getParam<MooseEnum>("equation"))
{
  for (unsigned int i = 0; i < _n_components; i++)
    _U.push_back(&adCoupledValue("variables", i));
}

Real
ADFlowBoundaryFlux1Phase::computeQpIntegral()
{
  std::vector<ADReal> U(_n_components);
  for (unsigned int i = 0; i < _n_components; i++)
    U[i] = (*_U[i])[_qp];

  const auto & flux = _boundary_uo.getFlux(_current_side, _current_elem->id(), U, _normals[_qp]);
  return MetaPhysicL::raw_value(flux[_equation_index]);
}
