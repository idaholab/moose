//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseAux);
registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseScalarAux);

template <typename T>
InputParameters
VolumeJunction1PhaseAuxTempl<T>::validParams()
{
  InputParameters params = T::validParams();

  params.addClassDescription("Computes various quantities for a VolumeJunction1Phase.");

  MooseEnum quantity("pressure temperature speed");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Which quantity to compute");
  params.addRequiredParam<Real>("volume", "Volume of the junction");
  params.addRequiredCoupledVar("rhoV", "rho*V of the junction");
  params.addRequiredCoupledVar("rhouV", "rho*u*V of the junction");
  params.addRequiredCoupledVar("rhovV", "rho*v*V of the junction");
  params.addRequiredCoupledVar("rhowV", "rho*w*V of the junction");
  params.addRequiredCoupledVar("rhoEV", "rho*E*V of the junction");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");

  return params;
}

template <>
VolumeJunction1PhaseAuxTempl<AuxKernel>::VolumeJunction1PhaseAuxTempl(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _volume(getParam<Real>("volume")),
    _rhoV(coupledValue("rhoV")),
    _rhouV(coupledValue("rhouV")),
    _rhovV(coupledValue("rhovV")),
    _rhowV(coupledValue("rhowV")),
    _rhoEV(coupledValue("rhoEV")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

template <>
VolumeJunction1PhaseAuxTempl<AuxScalarKernel>::VolumeJunction1PhaseAuxTempl(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _volume(getParam<Real>("volume")),
    _rhoV(coupledScalarValue("rhoV")),
    _rhouV(coupledScalarValue("rhouV")),
    _rhovV(coupledScalarValue("rhovV")),
    _rhowV(coupledScalarValue("rhowV")),
    _rhoEV(coupledScalarValue("rhoEV")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

template <typename T>
Real
VolumeJunction1PhaseAuxTempl<T>::computeValue()
{
  Real vJ, dvJ_drhoV;
  THM::v_from_rhoA_A(_rhoV[0], _volume, vJ, dvJ_drhoV);

  const RealVectorValue vel(_rhouV[0] / _rhoV[0], _rhovV[0] / _rhoV[0], _rhowV[0] / _rhoV[0]);
  const Real eJ = _rhoEV[0] / _rhoV[0] - 0.5 * vel * vel;

  switch (_quantity)
  {
    case Quantity::PRESSURE:
      return _fp.p_from_v_e(vJ, eJ);
      break;
    case Quantity::TEMPERATURE:
      return _fp.T_from_v_e(vJ, eJ);
      break;
    case Quantity::SPEED:
      return vel.norm();
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}

template class VolumeJunction1PhaseAuxTempl<AuxKernel>;
template class VolumeJunction1PhaseAuxTempl<AuxScalarKernel>;
