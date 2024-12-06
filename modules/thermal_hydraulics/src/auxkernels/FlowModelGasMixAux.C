//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelGasMixAux.h"
#include "FlowModelGasMixUtils.h"
#include "THMIndicesGasMix.h"

registerMooseObject("ThermalHydraulicsApp", FlowModelGasMixAux);

InputParameters
FlowModelGasMixAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  MooseEnum quantity("p T");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Quantity to compute");

  params.addRequiredCoupledVar("xirhoA", "xi*rho*A variable");
  params.addRequiredCoupledVar("rhoA", "rho*A variable");
  params.addRequiredCoupledVar("rhouA", "rho*u*A variable");
  params.addRequiredCoupledVar("rhoEA", "rho*E*A variable");
  params.addRequiredCoupledVar("area", "Cross-sectional area variable");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The IdealRealGasMixtureFluidProperties object");

  params.addClassDescription("Computes various quantities for FlowModelGasMix.");

  return params;
}

FlowModelGasMixAux::FlowModelGasMixAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _xirhoA(coupledValue("xirhoA")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),
    _area(coupledValue("area")),
    _fp(getUserObject<IdealRealGasMixtureFluidProperties>("fluid_properties"))
{
}

Real
FlowModelGasMixAux::computeValue()
{
  std::vector<Real> U(THMGasMix1D::N_FLUX_INPUTS);
  U[THMGasMix1D::XIRHOA] = _xirhoA[_qp];
  U[THMGasMix1D::RHOA] = _rhoA[_qp];
  U[THMGasMix1D::RHOUA] = _rhouA[_qp];
  U[THMGasMix1D::RHOEA] = _rhoEA[_qp];
  U[THMGasMix1D::AREA] = _area[_qp];

  const auto W = FlowModelGasMixUtils::computePrimitiveSolution<false>(U, _fp);

  switch (_quantity)
  {
    case Quantity::PRESSURE:
    {
      return W[THMGasMix1D::PRESSURE];
      break;
    }
    case Quantity::TEMPERATURE:
    {
      return W[THMGasMix1D::TEMPERATURE];
      break;
    }
    default:
      mooseAssert(false, "Invalid 'quantity' parameter.");
      return 0;
  }
}
