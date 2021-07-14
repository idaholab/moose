//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCStagnationInletBC.h"
#include "IdealGasFluidProperties.h"
#include "NS.h"

InputParameters
CNSFVHLLCStagnationInletBC::validParams()
{
  InputParameters params = CNSFVHLLCBC::validParams();
  params.addRequiredParam<PostprocessorName>("stagnation_temperature",
                                             "Specified inlet stagnation temperature.");
  params.addRequiredParam<PostprocessorName>("stagnation_pressure",
                                             "Specified inlet stagnation pressure.");
  return params;
}

CNSFVHLLCStagnationInletBC::CNSFVHLLCStagnationInletBC(const InputParameters & parameters)
  : CNSFVHLLCBC(parameters),
    _stagnation_temperature(this->getPostprocessorValue("stagnation_temperature")),
    _stagnation_pressure(this->getPostprocessorValue("stagnation_pressure")),
    _cp(getADMaterialProperty<Real>(NS::cp)),
    _cv(getADMaterialProperty<Real>(NS::cv))
{
  // we need to distinguish between ideal gas and non-ideal gas
  const IdealGasFluidProperties * fluid_ideal_gas =
      dynamic_cast<const IdealGasFluidProperties *>(&_fluid);
  if (!fluid_ideal_gas)
    paramError(
        NS::fluid,
        "Navier-Stokes module supports stagnation inlet BCs only for IdealGasFluidProperties. "
        "Non-ideal "
        "fluid "
        "properties do not implement the necessary interfaces to support isentropic processes.");
}

void
CNSFVHLLCStagnationInletBC::preComputeWaveSpeed()
{
  _normal_speed_boundary = _normal_speed_elem;
  _vel_boundary = _vel_elem[_qp];

  // for convenience compute the square of the speed
  ADReal speed_sq = _speed_elem[_qp] * _speed_elem[_qp];

  // Compute inlet temperature
  ADReal T_inlet = _stagnation_temperature - 0.5 * speed_sq / _cp[_qp];

  // Compute inlet pressure using isentropic relation
  ADReal gamma = _cp[_qp] / _cv[_qp];
  _p_boundary =
      _stagnation_pressure * std::pow(_stagnation_temperature / T_inlet, -gamma / (gamma - 1.));

  // Compute total energy from stagnation values.
  _specific_internal_energy_boundary = _cv[_qp] * T_inlet + 0.5 * speed_sq;

  _rho_boundary = _fluid.rho_from_p_T(_p_boundary, T_inlet);
  _ht_boundary = _specific_internal_energy_boundary + _p_boundary / _rho_boundary;
}
