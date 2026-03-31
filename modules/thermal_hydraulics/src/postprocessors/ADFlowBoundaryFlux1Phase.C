//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFlowBoundaryFlux1Phase.h"
#include "ADBoundaryFluxBase.h"
#include "THMIndicesVACE.h"

registerMooseObject("ThermalHydraulicsApp", ADFlowBoundaryFlux1Phase);

InputParameters
ADFlowBoundaryFlux1Phase::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  MooseEnum equation("mass=0 momentum=1 energy=2 passive=3");
  params.addRequiredParam<MooseEnum>(
      "equation", equation, "Equation for which to query flux vector");
  params.addParam<unsigned int>("passive_index",
                                "Index within list of passives if setting 'equation = passive'");

  params.addCoupledVar("A_linear", "Cross-sectional area, linear");
  params.set<std::vector<VariableName>>("A_linear") = {"A_linear"};

  params.addClassDescription("Retrieves an entry of a boundary flux vector for a 1-phase channel");

  return params;
}

ADFlowBoundaryFlux1Phase::ADFlowBoundaryFlux1Phase(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _A_linear(adCoupledValue("A_linear")),
    _rhoA(getADMaterialProperty<Real>("rhoA")),
    _rhouA(getADMaterialProperty<Real>("rhouA")),
    _rhoEA(getADMaterialProperty<Real>("rhoEA")),
    _passives_times_area(getADMaterialProperty<std::vector<Real>>("passives_times_area")),
    _boundary_name(getParam<std::vector<BoundaryName>>("boundary")[0]),
    _boundary_uo_name(_boundary_name + ":boundary_uo"),
    _boundary_uo(getUserObjectByName<ADBoundaryFluxBase>(_boundary_uo_name)),
    _equation_index(getEquationIndex())
{
}

Real
ADFlowBoundaryFlux1Phase::computeQpIntegral()
{
  const auto & passives_times_area = _passives_times_area[_qp];
  const unsigned int n_passives = passives_times_area.size();

  std::vector<ADReal> U(THMVACE1D::N_FLUX_INPUTS + n_passives, 0);
  U[THMVACE1D::RHOA] = _rhoA[_qp];
  U[THMVACE1D::RHOUA] = _rhouA[_qp];
  U[THMVACE1D::RHOEA] = _rhoEA[_qp];
  U[THMVACE1D::AREA] = _A_linear[_qp];
  for (const auto i : index_range(passives_times_area))
    U[THMVACE1D::N_FLUX_INPUTS + i] = passives_times_area[i];

  const auto & flux = _boundary_uo.getFlux(_current_side, _current_elem->id(), U, _normals[_qp]);
  mooseAssert(_equation_index < flux.size(), "Invalid equation index");
  return MetaPhysicL::raw_value(flux[_equation_index]);
}

unsigned int
ADFlowBoundaryFlux1Phase::getEquationIndex() const
{
  const auto equation = getParam<MooseEnum>("equation");
  if (equation == "mass")
    return THMVACE1D::MASS;
  else if (equation == "momentum")
    return THMVACE1D::MOMENTUM;
  else if (equation == "energy")
    return THMVACE1D::ENERGY;
  else if (equation == "passive")
  {
    if (!isParamValid("passive_index"))
      mooseError("If 'equation' is set to 'passive', then 'passive_index' must be supplied");
    else
    {
      const auto passive_index = getParam<unsigned int>("passive_index");
      return THMVACE1D::N_FLUX_OUTPUTS + passive_index;
    }
  }
  else
    mooseError("Invalid 'equation' value.");
}
