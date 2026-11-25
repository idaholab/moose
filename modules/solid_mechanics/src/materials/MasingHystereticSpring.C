//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MasingHystereticSpring.h"

#include "Function.h"

registerMooseObject("SolidMechanicsApp", MasingHystereticSpring);

InputParameters
MasingHystereticSpring::validParams()
{
  InputParameters params = SpringLawBase::validParams();
  params.addClassDescription("Hysteretic spring law: the backbone curve on first loading and "
                             "the Masing rule on every unloading and reloading branch.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "backbone_functions",
      "Three functions, one per local axis, giving the force (dimension force, or force per "
      "area when tributary_area is given; moment in the rotational mode) on first loading as a "
      "function of the deformation along that axis (dimension length, or radian), the x "
      "coordinate of the function (a PiecewiseLinear function needs axis = x). The Masing rule "
      "assumes a backbone that is odd in the deformation: f(-x) = -f(x).");
  return params;
}

MasingHystereticSpring::MasingHystereticSpring(const InputParameters & parameters)
  : SpringLawBase(parameters),
    _force_per_area(declareProperty<RealVectorValue>(name() + "_force")),
    _force_per_area_old(getMaterialPropertyOld<RealVectorValue>(name() + "_force")),
    _deformation_old_state(declareProperty<RealVectorValue>(name() + "_deformation")),
    _deformation_old(getMaterialPropertyOld<RealVectorValue>(name() + "_deformation")),
    _direction(declareProperty<RealVectorValue>(name() + "_direction")),
    _direction_old(getMaterialPropertyOld<RealVectorValue>(name() + "_direction")),
    _on_backbone(declareProperty<RealVectorValue>(name() + "_on_backbone")),
    _on_backbone_old(getMaterialPropertyOld<RealVectorValue>(name() + "_on_backbone")),
    _branch_side(declareProperty<RealVectorValue>(name() + "_branch_side")),
    _branch_side_old(getMaterialPropertyOld<RealVectorValue>(name() + "_branch_side")),
    _reversal_deformation(declareProperty<RealVectorValue>(name() + "_reversal_deformation")),
    _reversal_deformation_old(
        getMaterialPropertyOld<RealVectorValue>(name() + "_reversal_deformation")),
    _reversal_force(declareProperty<RealVectorValue>(name() + "_reversal_force")),
    _reversal_force_old(getMaterialPropertyOld<RealVectorValue>(name() + "_reversal_force"))
{
  const auto & names = getParam<std::vector<FunctionName>>("backbone_functions");
  if (names.size() != 3)
    paramError("backbone_functions", "Three functions are needed, one per local axis.");
  for (const auto & name : names)
    _backbones.push_back(&getFunctionByName(name));
}

void
MasingHystereticSpring::initQpStatefulProperties()
{
  _force_per_area[_qp].zero();
  _deformation_old_state[_qp].zero();
  _direction[_qp].zero();
  _on_backbone[_qp] = RealVectorValue(1.0, 1.0, 1.0);
  _branch_side[_qp].zero();
  _reversal_deformation[_qp].zero();
  _reversal_force[_qp].zero();
}

void
MasingHystereticSpring::computeLaw(const RealVectorValue & deformation,
                                   RealVectorValue & force,
                                   RankTwoTensor & stiffness)
{
  stiffness.zero();
  for (unsigned int axis = 0; axis < 3; ++axis)
  {
    const Function & backbone = *_backbones[axis];
    const Real x = deformation(axis);
    const Real x_old = _deformation_old[_qp](axis);
    const Real increment = x - x_old;

    // The loading direction of this step; an unchanged deformation keeps the old direction.
    Real direction = _direction_old[_qp](axis);
    if (!MooseUtils::absoluteFuzzyEqual(increment, 0.0))
      direction = increment > 0.0 ? 1.0 : -1.0;

    // A change of direction starts a new branch from the last converged point.
    Real reversal_deformation = _reversal_deformation_old[_qp](axis);
    Real reversal_force = _reversal_force_old[_qp](axis);
    bool on_backbone = _on_backbone_old[_qp](axis) > 0.5;
    Real branch_side = _branch_side_old[_qp](axis);
    const Real direction_old = _direction_old[_qp](axis);
    if (direction_old != 0.0 && direction != direction_old)
    {
      reversal_deformation = x_old;
      reversal_force = _force_per_area_old[_qp](axis);
      on_backbone = false;
      branch_side = 0.0;
    }

    const Point on_curve(x, 0.0, 0.0);
    const Real backbone_force = backbone.value(_t, on_curve);
    if (!on_backbone)
    {
      // Masing rule: the branch is the backbone scaled by two about the reversal point. It
      // runs on one side of the backbone (below it when unloading from the backbone, above
      // it when reloading) and ends where it crosses to the other side; from there the
      // backbone is followed again.
      const Point half_way((x - reversal_deformation) / 2.0, 0.0, 0.0);
      const Real branch_force = reversal_force + 2.0 * backbone.value(_t, half_way);
      const Real offset = branch_force - backbone_force;
      if (branch_side == 0.0 && !MooseUtils::absoluteFuzzyEqual(offset, 0.0))
        branch_side = offset > 0.0 ? 1.0 : -1.0;
      if (branch_side * offset < 0.0)
        on_backbone = true;
      else
      {
        force(axis) = branch_force;
        stiffness(axis, axis) = backbone.gradient(_t, half_way)(0);
      }
    }
    if (on_backbone)
    {
      force(axis) = backbone_force;
      stiffness(axis, axis) = backbone.gradient(_t, on_curve)(0);
    }

    _force_per_area[_qp](axis) = force(axis);
    _deformation_old_state[_qp](axis) = x;
    _direction[_qp](axis) = direction;
    _on_backbone[_qp](axis) = on_backbone ? 1.0 : 0.0;
    _branch_side[_qp](axis) = on_backbone ? 0.0 : branch_side;
    _reversal_deformation[_qp](axis) = reversal_deformation;
    _reversal_force[_qp](axis) = reversal_force;
  }
}
