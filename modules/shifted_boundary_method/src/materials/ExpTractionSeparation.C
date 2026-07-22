//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpTractionSeparation.h"
#include "BoundaryShortestDistanceToSurface.h"

registerMooseObject("ShiftedBoundaryMethodApp", ExpTractionSeparation);

InputParameters
ExpTractionSeparation::validParams()
{
  InputParameters params = ADCZMComputeLocalTractionTotalBase::validParams();

  params.addRequiredParam<Real>("Gc", "Fracture energy G_c.");
  params.addRequiredParam<Real>("delta0", "Softening length scale delta_0.");
  params.addRequiredParam<Real>("beta", "Tangential weighting beta in delta_eff.");

  // Small stabilizer in norms
  params.addParam<Real>("eps", 1e-16, "Small stabilizer for norms.");

  params.addParam<bool>("irreversible_damage", false, "If true, damage is irreversible.");

  params.addClassDescription("Exponential traction separation law.");
  return params;
}

ExpTractionSeparation::ExpTractionSeparation(const InputParameters & parameters)
  : ADCZMComputeLocalTractionTotalBase(parameters),
    _Gc(getParam<Real>("Gc")),
    _delta0(getParam<Real>("delta0")),
    _beta(getParam<Real>("beta")),
    _eps(getParam<Real>("eps")),
    _irreversible_damage(getParam<bool>("irreversible_damage")),
    _interface_displacement_jump(
        getADMaterialPropertyByName<RealVectorValue>("interface_displacement_jump")),
    _interface_effective_displacement_jump(
        declareADPropertyByName<RealVectorValue>("interface_effective_displacement_jump")),
    _effective_displacement_jump_scalar_max(
        declareADPropertyByName<Real>("effective_displacement_jump_max_scalar")),
    _effective_displacement_jump_scalar_max_old(
        getMaterialPropertyOldByName<Real>("effective_displacement_jump_max_scalar")),
    _damage(declareADPropertyByName<Real>("damage"))
{
}

void
ExpTractionSeparation::initQpStatefulProperties()
{
  _effective_displacement_jump_scalar_max[_qp] = 0.0;
}

void
ExpTractionSeparation::computeInterfaceTraction()
{
  const auto jump_on_GP = _interface_displacement_jump[_qp];

  // Normal and tangential components of jump
  const auto delta_n = jump_on_GP(0);
  const auto delta_t_sq = jump_on_GP(1) * jump_on_GP(1) + jump_on_GP(2) * jump_on_GP(2);
  const auto delta_n_sq = delta_n * delta_n;

  // Use the tangential norm squared directly. The intermediate sqrt(delta_t_sq) carries a
  // singular derivative at zero tangential jump, which can poison the exact AD Jacobian even
  // though the final effective jump only depends on delta_t^2.
  auto delta_eff = sqrt(delta_n_sq + _beta * delta_t_sq + _eps);

  _interface_effective_displacement_jump[_qp] =
      RealVectorValue(raw_value(delta_n),
                      std::sqrt(_beta) * raw_value(jump_on_GP(1)),
                      std::sqrt(_beta) * raw_value(jump_on_GP(2)));

  if (_irreversible_damage)
  {
    if (_effective_displacement_jump_scalar_max_old[_qp] < delta_eff.value())
      _effective_displacement_jump_scalar_max[_qp] = delta_eff; // update max
    else
      delta_eff = ADReal(_effective_displacement_jump_scalar_max_old[_qp]); // use old max
  }

  // Damage evaluation
  const auto d_cur = 1.0 - exp(-delta_eff / _delta0); // the definition of damage
  _damage[_qp] = d_cur;

  // Compute traction
  const auto c = _Gc / (_delta0 * _delta0);
  const auto one_minus_d = 1.0 - d_cur;
  _interface_traction[_qp] = one_minus_d * c * jump_on_GP;
}
