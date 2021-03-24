//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldFractureBase.h"
#include "MooseUtils.h"

InputParameters
PhaseFieldFractureBase::validParams()
{
  InputParameters params = DamageBase::validParams();
  params.addClassDescription(
      "The entry point for using a phase-field fracture model as a damage model.");
  params.addRequiredCoupledVar("c", "Name of the phase-field (damage) variable");
  params.addParam<MaterialPropertyName>(
      "elastic_energy", "E_el", "Name of the elastic energy material");
  params.addParam<MaterialPropertyName>(
      "degradation_function", "g", "Name of the degradation function material");

  params.addParam<bool>(
      "use_old_elastic_energy",
      false,
      "Whether to use the elastic energy from the previous time step to drive damage evolution. "
      "Setting this option to true should significantly improve the convergence, but the solution "
      "is only valid when the time step size is sufficiently small.");
  params.addRangeCheckedParam<Real>(
      "maximum_damage_increment",
      0.1,
      "maximum_damage_increment>0 & maximum_damage_increment<1",
      "Maximum damage increment allowed for simulations with adaptive time step");
  params.addParam<bool>("hybrid",
                        false,
                        "The hybrid formulation only computes the active strain energy using the "
                        "specific split and always uses isotropic degradation for stress.");
  params.addParamNamesToGroup("use_old_elastic_energy maximum_damage_increment hybrid", "Advanced");

  return params;
}

PhaseFieldFractureBase::PhaseFieldFractureBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<DamageBase>(parameters),
    // Bulk properties and parameters
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),
    _c(coupledValue("c")),
    _c_old(coupledValueOld("c")),
    _c_name(getVar("c", 0)->name()),

    // The elastic energy and its derivatives
    _E_name(getParam<MaterialPropertyName>(_base_name + "elastic_energy")),
    _E(declareProperty<Real>(_E_name)),
    _E_active(declareProperty<Real>(_E_name + "_active")),
    _E_active_old(getMaterialPropertyOldByName<Real>(_E_name + "_active")),
    _dE_dc(declarePropertyDerivative<Real>(_E_name, _c_name)),
    _d2E_dc2(declarePropertyDerivative<Real>(_E_name, _c_name, _c_name)),
    _d2E_dcdstrain(declareProperty<RankTwoTensor>(_base_name + "d2Fdcdstrain")),

    // The degradation function and its derivatives
    _g_name(getParam<MaterialPropertyName>(_base_name + "degradation_function")),
    _g(getMaterialProperty<Real>(_g_name)),
    _dg_dc(getMaterialPropertyDerivative<Real>(_g_name, _c_name)),
    _d2g_dc2(getMaterialPropertyDerivative<Real>(_g_name, _c_name, _c_name)),

    // Strains and stresses
    _elastic_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "elastic_strain")),
    _undamaged_stress(declareProperty<RankTwoTensor>(_base_name + "undamaged_stress")),
    _undamaged_stress_old(
        getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "undamaged_stress")),
    _stress_pos(declareProperty<RankTwoTensor>(_base_name + "stress_positive")),
    _dstress_dc(declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", _c_name)),

    // Algorithmic controls
    _use_old_elastic_energy(getParam<bool>("use_old_elastic_energy")),
    _maximum_damage_increment(getParam<Real>("maximum_damage_increment")),
    _hybrid(getParam<bool>("hybrid"))
{
}

void
PhaseFieldFractureBase::initQpStatefulProperties()
{
  _undamaged_stress[_qp].zero();
  _E_active[_qp] = 0;
}

void
PhaseFieldFractureBase::updateStressForDamage(RankTwoTensor & stress_new)
{
  _undamaged_stress[_qp] = stress_new;
  computeDamagedStress(stress_new);
  computeElasticEnergy();
}

void
PhaseFieldFractureBase::computeUndamagedOldStress(RankTwoTensor & stress_old)
{
  stress_old = _undamaged_stress_old[_qp];
}

Real
PhaseFieldFractureBase::computeTimeStepLimit()
{
  Real current_damage_increment = (_c[_qp] - _c_old[_qp]);

  if (MooseUtils::absoluteFuzzyEqual(current_damage_increment, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _maximum_damage_increment / current_damage_increment;
}

void
PhaseFieldFractureBase::finiteStrainRotation(const RankTwoTensor & rotation_increment)
{
  _undamaged_stress[_qp] =
      rotation_increment * _undamaged_stress[_qp] * rotation_increment.transpose();
  _dstress_dc[_qp] = rotation_increment * _dstress_dc[_qp] * rotation_increment.transpose();
}

Real
PhaseFieldFractureBase::Macaulay(const Real x, const bool deriv) const
{
  if (deriv)
    return x > 0 ? 1 : 0;
  return 0.5 * (x + std::abs(x));
}

std::vector<Real>
PhaseFieldFractureBase::Macaulay(const std::vector<Real> & v, const bool deriv) const
{
  std::vector<Real> m = v;
  for (auto & x : m)
    x = Macaulay(x, deriv);
  return m;
}
