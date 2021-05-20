//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVStagnationInletBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "IdealGasFluidProperties.h"

namespace nms = NS;

InputParameters
CNSFVStagnationInletBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredParam<PostprocessorName>("stagnation_temperature",
                                             "Specified inlet stagnation temperature.");
  params.addRequiredParam<PostprocessorName>("stagnation_pressure",
                                             "Specified inlet stagnation pressure.");
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
  params.addParam<Real>(NS::porosity, 1, "porosity");
  return params;
}

CNSFVStagnationInletBC::CNSFVStagnationInletBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _eps(getParam<Real>(nms::porosity)),
    _stagnation_temperature(this->getPostprocessorValue("stagnation_temperature")),
    _stagnation_pressure(this->getPostprocessorValue("stagnation_pressure")),
    _velocity(getADMaterialProperty<RealVectorValue>(nms::velocity)),
    _speed(getADMaterialProperty<Real>(nms::speed)),
    _cp(getADMaterialProperty<Real>(nms::cp)),
    _cv(getADMaterialProperty<Real>(nms::cv)),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(nms::fluid))
{
  // we need to distinguish between ideal gas and non-ideal gas
  _fluid_ideal_gas = dynamic_cast<const IdealGasFluidProperties *>(&_fluid);
}

void
CNSFVStagnationInletBC::inletConditionHelper(ADReal & p_inlet,
                                             ADReal & T_inlet,
                                             ADReal & rho_inlet,
                                             ADReal & H_inlet) const
{
  if (!_fluid_ideal_gas)
    paramError(
        nms::fluid,
        "Navier-Stokes module supports stagnation inlet BCs only for IdealGasFluidProperties. "
        "Non-ideal "
        "fluid "
        "properties do not implement the necessary interfaces to support isentropic processes.");

  // total internal energy
  ADReal E_inlet;

  // for convenience compute the square of the speed
  ADReal speed_sq = _speed[_qp] * _speed[_qp];

  // Compute inlet temperature
  T_inlet = _stagnation_temperature - 0.5 * speed_sq / _cp[_qp];

  // Compute inlet pressure using isentropic relation
  ADReal gamma = _cp[_qp] / _cv[_qp];
  p_inlet =
      _stagnation_pressure * std::pow(_stagnation_temperature / T_inlet, -gamma / (gamma - 1.));

  // Compute total energy from stagnation values.
  E_inlet = _cv[_qp] * T_inlet + 0.5 * speed_sq;

  rho_inlet = _fluid.rho_from_p_T(p_inlet, T_inlet);
  H_inlet = E_inlet + p_inlet / rho_inlet;
}
