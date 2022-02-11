//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseTemperatureAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseTemperatureAux);

InputParameters
VolumeJunction1PhaseTemperatureAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<Real>("volume", "Volume of the junction");
  params.addRequiredCoupledVar("rhoV", "rho*V of the junction");
  params.addRequiredCoupledVar("rhouV", "rho*u*V of the junction");
  params.addRequiredCoupledVar("rhovV", "rho*v*V of the junction");
  params.addRequiredCoupledVar("rhowV", "rho*w*V of the junction");
  params.addRequiredCoupledVar("rhoEV", "rho*E*V of the junction");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  params.addClassDescription("Computes temperature from the 1-phase volume junction variables.");
  return params;
}

VolumeJunction1PhaseTemperatureAux::VolumeJunction1PhaseTemperatureAux(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _volume(getParam<Real>("volume")),
    _rhoV(coupledScalarValue("rhoV")),
    _rhouV(coupledScalarValue("rhouV")),
    _rhovV(coupledScalarValue("rhovV")),
    _rhowV(coupledScalarValue("rhowV")),
    _rhoEV(coupledScalarValue("rhoEV")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
VolumeJunction1PhaseTemperatureAux::computeValue()
{
  Real vJ, dvJ_drhoV;
  THM::v_from_rhoA_A(_rhoV[0], _volume, vJ, dvJ_drhoV);

  const RealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);
  const Real rhouV2 = rhouV_vec * rhouV_vec;
  const Real eJ = _rhoEV[0] / _rhoV[0] - 0.5 * rhouV2 / (_rhoV[0] * _rhoV[0]);

  return _fp.T_from_v_e(vJ, eJ);
}
