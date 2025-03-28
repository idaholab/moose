//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelGasMixIC.h"
#include "Function.h"
#include "FlowModelGasMixUtils.h"
#include "THMIndicesGasMix.h"

registerMooseObject("ThermalHydraulicsApp", FlowModelGasMixIC);

InputParameters
FlowModelGasMixIC::validParams()
{
  InputParameters params = InitialCondition::validParams();

  MooseEnum quantity("rho rhoEA");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Quantity to compute");

  params.addRequiredParam<FunctionName>("mass_fraction", "Secondary gas mass fraction function");
  params.addRequiredParam<FunctionName>("pressure", "Pressure function");
  params.addRequiredParam<FunctionName>("temperature", "Temperature function");
  params.addRequiredParam<FunctionName>("velocity", "Velocity function");
  params.addRequiredCoupledVar("area", "Cross-sectional area variable");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The VaporMixtureFluidProperties object");

  params.addClassDescription("IC for the solution variables of FlowModelGasMix.");

  return params;
}

FlowModelGasMixIC::FlowModelGasMixIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _xi(getFunction("mass_fraction")),
    _p(getFunction("pressure")),
    _T(getFunction("temperature")),
    _vel(getFunction("velocity")),
    _area(coupledValue("area")),
    _fp(getUserObject<VaporMixtureFluidProperties>("fluid_properties"))
{
}

Real
FlowModelGasMixIC::value(const Point & pt)
{
  const auto xi = _xi.value(_t, pt);
  const auto p = _p.value(_t, pt);
  const auto T = _T.value(_t, pt);

  const auto v = _fp.v_from_p_T(p, T, {xi});
  const auto rho = 1.0 / v;

  switch (_quantity)
  {
    case Quantity::DENSITY:
    {
      return rho;
      break;
    }
    case Quantity::RHOEA:
    {
      const auto vel = _vel.value(_t, pt);
      const auto e = _fp.e_from_p_T(p, T, {xi});
      const auto E = e + 0.5 * vel * vel;
      return rho * E * _area[_qp];
      break;
    }
    default:
      mooseAssert(false, "Invalid 'quantity' parameter.");
      return 0;
  }
}
