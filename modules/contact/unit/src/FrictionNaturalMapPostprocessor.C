//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralPostprocessor.h"
#include "LMWeightedVelocitiesUserObject.h"
#include "MooseVariableFieldBase.h"
#include "MortarContactUtils.h"

class FrictionNaturalMapPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  FrictionNaturalMapPostprocessor(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  PostprocessorValue getValue() const override;

private:
  const LMWeightedVelocitiesUserObject & _weighted_velocities_uo;
  const MooseVariableFieldBase & _normal_lm;
  const MooseVariableFieldBase & _tangential_lm;
  const MooseVariableFieldBase * const _tangential_lm_two;
  const Real _friction_coefficient;
  Real _value = 0;
};

registerMooseObject("ContactApp", FrictionNaturalMapPostprocessor);

InputParameters
FrictionNaturalMapPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the formulation-independent Coulomb natural-map error for unit testing.");
  params.addRequiredParam<UserObjectName>("weighted_velocities_uo",
                                          "The LM weighted-velocities user object.");
  params.addRequiredParam<VariableName>("normal_lm", "The normal contact multiplier.");
  params.addRequiredParam<VariableName>("tangential_lm", "The first tangential multiplier.");
  params.addParam<VariableName>("tangential_lm_two", "The optional second tangential multiplier.");
  params.addRequiredParam<Real>("friction_coefficient", "The Coulomb friction coefficient.");
  return params;
}

FrictionNaturalMapPostprocessor::FrictionNaturalMapPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _weighted_velocities_uo(
        getUserObject<LMWeightedVelocitiesUserObject>("weighted_velocities_uo")),
    _normal_lm(_fe_problem.getVariable(_tid, getParam<VariableName>("normal_lm"))),
    _tangential_lm(_fe_problem.getVariable(_tid, getParam<VariableName>("tangential_lm"))),
    _tangential_lm_two(
        isParamValid("tangential_lm_two")
            ? &_fe_problem.getVariable(_tid, getParam<VariableName>("tangential_lm_two"))
            : nullptr),
    _friction_coefficient(getParam<Real>("friction_coefficient"))
{
}

void
FrictionNaturalMapPostprocessor::initialize()
{
  _value = 0;
}

void
FrictionNaturalMapPostprocessor::execute()
{
  const auto value = [](const DofObject * const dof, const MooseVariableFieldBase & variable)
  {
    const auto index = dof->dof_number(variable.sys().number(), variable.number(), 0);
    return (*variable.sys().currentSolution())(index);
  };

  for (const auto & [dof, gap_data] : _weighted_velocities_uo.dofToWeightedGap())
  {
    const auto c_it = _weighted_velocities_uo.dofToDerivedC().find(dof);
    if (c_it == _weighted_velocities_uo.dofToDerivedC().end())
      mooseError("Natural-map postprocessor is missing a physical contact scale.");
    const auto velocity_it = _weighted_velocities_uo.dofToWeightedVelocities().find(dof);
    if (velocity_it == _weighted_velocities_uo.dofToWeightedVelocities().end())
      mooseError("Natural-map postprocessor is missing a tangential velocity.");

    const Real normalization = gap_data.second;
    const Real contact_scale = c_it->second[0];
    const Real gap = MetaPhysicL::raw_value(gap_data.first) / normalization;
    const Real normal_pressure = value(dof, _normal_lm);
    // Edge dropping zeros inactive multipliers even when their diagnostic gap data overlap.
    if (normal_pressure == 0.0 && gap < 0.0)
      continue;
    const auto augmented_normal =
        Moose::Mortar::Contact::augmentedNormalPressure(normal_pressure, contact_scale * gap);
    const auto radius =
        Moose::Mortar::Contact::coulombFrictionRadius(_friction_coefficient, augmented_normal);
    std::array<Real, 2> tangential_pressure = {{value(dof, _tangential_lm), 0}};
    if (_tangential_lm_two)
      tangential_pressure[1] = value(dof, *_tangential_lm_two);
    std::array<Real, 2> augmented_tangential;
    for (const auto i : index_range(augmented_tangential))
      augmented_tangential[i] =
          tangential_pressure[i] + contact_scale * MetaPhysicL::raw_value(velocity_it->second[i]) *
                                       _fe_problem.dt() / normalization;
    const auto friction_residual = Moose::Mortar::Contact::alartCurnierFrictionResidual(
        tangential_pressure, augmented_tangential, radius);

    _value = std::max(_value, std::abs(std::min(normal_pressure, contact_scale * gap)));
    _value = std::max(_value, Moose::Mortar::Contact::tangentialNorm(friction_residual));
  }
}

PostprocessorValue
FrictionNaturalMapPostprocessor::getValue() const
{
  return _value;
}
