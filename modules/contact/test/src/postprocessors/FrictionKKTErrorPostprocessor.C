//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "GeneralPostprocessor.h"
#include "LMWeightedVelocitiesUserObject.h"
#include "MooseVariableFieldBase.h"

#include <algorithm>
#include <cmath>

/** Computes a formulation-independent Coulomb KKT error for mortar contact tests. */
class FrictionKKTErrorPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  FrictionKKTErrorPostprocessor(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  PostprocessorValue getValue() const override;

private:
  const LMWeightedVelocitiesUserObject & _weighted_velocities_uo;
  const MooseVariableFieldBase & _normal_lm;
  const MooseVariableFieldBase & _tangential_lm;
  const Real _friction_coefficient;
  const Real _normal_scale;
  const Real _tangential_scale;
  Real _value = 0;
};

registerMooseObject("ContactTestApp", FrictionKKTErrorPostprocessor);

InputParameters
FrictionKKTErrorPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Computes a formulation-independent Coulomb KKT error for mortar contact tests.");
  params.addRequiredParam<UserObjectName>("weighted_velocities_uo",
                                          "The LM weighted-velocities user object.");
  params.addRequiredParam<VariableName>("normal_lm", "The normal contact multiplier.");
  params.addRequiredParam<VariableName>("tangential_lm", "The tangential contact multiplier.");
  params.addRequiredParam<Real>("friction_coefficient", "The Coulomb friction coefficient.");
  params.addRequiredRangeCheckedParam<Real>(
      "normal_scale", "normal_scale > 0", "The physical normal pressure scale.");
  params.addRequiredRangeCheckedParam<Real>(
      "tangential_scale", "tangential_scale > 0", "The physical tangential pressure scale.");
  return params;
}

FrictionKKTErrorPostprocessor::FrictionKKTErrorPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _weighted_velocities_uo(
        getUserObject<LMWeightedVelocitiesUserObject>("weighted_velocities_uo")),
    _normal_lm(_fe_problem.getVariable(_tid, getParam<VariableName>("normal_lm"))),
    _tangential_lm(_fe_problem.getVariable(_tid, getParam<VariableName>("tangential_lm"))),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _normal_scale(getParam<Real>("normal_scale")),
    _tangential_scale(getParam<Real>("tangential_scale"))
{
}

void
FrictionKKTErrorPostprocessor::initialize()
{
  _value = 0;
}

void
FrictionKKTErrorPostprocessor::execute()
{
  const auto value = [](const DofObject * const dof, const MooseVariableFieldBase & variable)
  {
    const auto index = dof->dof_number(variable.sys().number(), variable.number(), 0);
    return (*variable.sys().currentSolution())(index);
  };

  for (const auto & [dof, gap_data] : _weighted_velocities_uo.dofToWeightedGap())
  {
    if (dof->processor_id() != processor_id())
      continue;

    const auto velocity_it = _weighted_velocities_uo.dofToWeightedVelocities().find(dof);
    if (velocity_it == _weighted_velocities_uo.dofToWeightedVelocities().end())
      mooseError("KKT postprocessor is missing a tangential velocity.");

    const Real normalization = gap_data.second;
    const Real gap = MetaPhysicL::raw_value(gap_data.first) / normalization;
    const Real normal_pressure = value(dof, _normal_lm);
    // Edge dropping zeros inactive multipliers even when their diagnostic gap data overlap.
    if (normal_pressure == 0.0 && gap < 0.0)
      continue;
    const Real tangential_pressure = value(dof, _tangential_lm);
    const Real augmented_tangential =
        tangential_pressure + _tangential_scale *
                                  MetaPhysicL::raw_value(velocity_it->second[0]) *
                                  _fe_problem.dt() / normalization;
    const Real radius = _friction_coefficient * std::max(0.0, normal_pressure);
    const Real projected_tangential =
        std::abs(augmented_tangential) <= radius
            ? augmented_tangential
            : std::copysign(radius, augmented_tangential);

    _value = std::max(_value, std::abs(std::min(normal_pressure, _normal_scale * gap)));
    _value = std::max(_value, std::abs(tangential_pressure - projected_tangential));
  }
}

void
FrictionKKTErrorPostprocessor::finalize()
{
  gatherMax(_value);
}

PostprocessorValue
FrictionKKTErrorPostprocessor::getValue() const
{
  return _value;
}
