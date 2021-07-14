//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC.h"
#include "NS.h"
#include "Function.h"
#include "SinglePhaseFluidProperties.h"

InputParameters
PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC::validParams()
{
  auto params = PCNSFVHLLCBC::validParams();
  params.addRequiredParam<FunctionName>(NS::superficial_momentum_x,
                                        "The x component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::superficial_momentum_y,
                                "The y component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::superficial_momentum_z,
                                "The z component of the inlet superficial momentum");
  params.addRequiredParam<FunctionName>(NS::temperature, "temperature specified as a function");
  return params;
}

PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC::PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC(
    const InputParameters & parameters)
  : PCNSFVHLLCBC(parameters),
    _superficial_rhou_boundary(getFunction(NS::superficial_momentum_x)),
    _superficial_rhov_boundary(isParamValid(NS::superficial_momentum_y)
                                   ? &getFunction(NS::superficial_momentum_y)
                                   : nullptr),
    _superficial_rhow_boundary(isParamValid(NS::superficial_momentum_z)
                                   ? &getFunction(NS::superficial_momentum_z)
                                   : nullptr),
    _temperature_boundary(getFunction(NS::temperature))
{
  if (_mesh.dimension() > 1 && !_superficial_rhov_boundary)
    mooseError("If the mesh dimension is greater than 1, a function for the y superficial momentum "
               "must be provided");
  if (_mesh.dimension() > 2 && !_superficial_rhow_boundary)
    mooseError("If the mesh dimension is greater than 2, a function for the z superficial momentum "
               "must be provided");
}

void
PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC::preComputeWaveSpeed()
{
  // rho implicit -> 1 numerical bc
  _rho_boundary = _rho_elem[_qp];

  _eps_boundary = _eps_elem[_qp];

  RealVectorValue mass_flux_boundary(
      _superficial_rhou_boundary.value(_t, _face_info->faceCentroid()), 0, 0);
  _vel_boundary.assign(
      ADRealVectorValue(mass_flux_boundary(0) / _rho_boundary / _eps_boundary, 0, 0));
  if (_superficial_rhov_boundary)
  {
    mass_flux_boundary(1) = _superficial_rhov_boundary->value(_t, _face_info->faceCentroid());
    _vel_boundary(1) = mass_flux_boundary(1) / _rho_boundary / _eps_boundary;
  }
  if (_superficial_rhow_boundary)
  {
    mass_flux_boundary(2) = _superficial_rhow_boundary->value(_t, _face_info->faceCentroid());
    _vel_boundary(2) = mass_flux_boundary(2) / _rho_boundary / _eps_boundary;
  }
  _normal_speed_boundary = _normal * _vel_boundary;

  const ADReal T_boundary = _temperature_boundary.value(_t, _face_info->faceCentroid());
  const ADReal v_boundary = 1 / _rho_boundary;
  _specific_internal_energy_boundary = _fluid.e_from_T_v(T_boundary, v_boundary);
  _et_boundary = _specific_internal_energy_boundary + 0.5 * _vel_boundary * _vel_boundary;
  _rho_et_boundary = _rho_boundary * _et_boundary;
  _pressure_boundary = _fluid.p_from_T_v(T_boundary, v_boundary);
  _ht_boundary = _et_boundary + _pressure_boundary / _rho_boundary;
}
