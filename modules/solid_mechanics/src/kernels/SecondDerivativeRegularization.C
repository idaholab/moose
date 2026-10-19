//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecondDerivativeRegularization.h"

registerMooseObject("SolidMechanicsApp", SecondDerivativeRegularization);
registerMooseObject("SolidMechanicsApp", ADSecondDerivativeRegularization);

namespace
{
MooseEnum
regularizationType()
{
  return MooseEnum("huhu lulu huhu_lulu");
}

SecondDerivativeRegularizationType
regularizationType(const MooseEnum & regularization)
{
  if (regularization == "huhu")
    return SecondDerivativeRegularizationType::Huhu;
  if (regularization == "lulu")
    return SecondDerivativeRegularizationType::Lulu;
  if (regularization == "huhu_lulu")
    return SecondDerivativeRegularizationType::HuhuLulu;

  mooseError("Unknown second-derivative regularization type '", regularization, "'.");
}

Real
defaultLuluFactor(const unsigned int dim)
{
  return 1.0 / static_cast<Real>(dim);
}

template <bool is_ad, typename Variable>
const GenericVariableSecond<is_ad> &
secondSln(Variable & var)
{
  if constexpr (is_ad)
    return var.adSecondSln();
  else
    return var.secondSln();
}

/// Shared weak-form contraction used by both the manual and AD kernels.
template <typename SolutionSecond, typename TestSecond>
auto
regularization(const SolutionSecond & second_u,
               const TestSecond & second_test,
               const SecondDerivativeRegularizationType type,
               const Real lulu_factor)
{
  switch (type)
  {
    case SecondDerivativeRegularizationType::Huhu:
      return second_u.contract(second_test);
    case SecondDerivativeRegularizationType::Lulu:
      return second_u.tr() * second_test.tr();
    case SecondDerivativeRegularizationType::HuhuLulu:
      // HuHu-LuLu keeps the Hessian contraction and subtracts a scaled Laplacian contraction.
      return second_u.contract(second_test) - lulu_factor * second_u.tr() * second_test.tr();
  }

  mooseError("Unknown second-derivative regularization type.");
}
}

template <bool is_ad>
InputParameters
SecondDerivativeRegularizationTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  if constexpr (is_ad)
    params.addClassDescription("Adds a second-derivative regularization term using HuHu, LuLu, "
                               "or HuHu-LuLu Hessian/Laplacian contractions. The Jacobian is "
                               "computed using automatic differentiation.");
  else
    params.addClassDescription("Adds a second-derivative regularization term using HuHu, LuLu, "
                               "or HuHu-LuLu Hessian/Laplacian contractions.");
  params.addRequiredParam<MooseEnum>(
      "regularization",
      regularizationType(),
      "Regularization type to apply: 'huhu' for Hessian-Hessian, 'lulu' for "
      "Laplacian-Laplacian, or 'huhu_lulu' for HuHu minus a scaled LuLu correction.");
  params.addRequiredRangeCheckedParam<Real>(
      "coefficient", "coefficient >= 0", "Coefficient multiplying the regularization term.");
  params.addParam<Real>("lulu_factor",
                        "Factor multiplying the LuLu correction for 'huhu_lulu'. If omitted, "
                        "defaults to 1 / dim, where dim is the mesh dimension.");
  return params;
}

template <bool is_ad>
SecondDerivativeRegularizationTempl<is_ad>::SecondDerivativeRegularizationTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _regularization_type(regularizationType(this->template getParam<MooseEnum>("regularization"))),
    _coefficient(this->template getParam<Real>("coefficient")),
    _dim(this->_mesh.dimension()),
    _lulu_factor(this->isParamValid("lulu_factor") ? this->template getParam<Real>("lulu_factor")
                                                   : defaultLuluFactor(_dim)),
    _second_u(secondSln<is_ad>(this->_var)),
    _second_test(this->secondTest()),
    _second_phi(this->_var.secondPhi())
{
  if (_regularization_type != SecondDerivativeRegularizationType::HuhuLulu &&
      this->isParamValid("lulu_factor"))
    this->paramError("lulu_factor", "Only used when 'regularization = huhu_lulu'.");

  if (_regularization_type == SecondDerivativeRegularizationType::HuhuLulu &&
      _lulu_factor > defaultLuluFactor(_dim))
  {
    // The bound c_L <= 1 / dim keeps the HuHu-LuLu quadratic form nonnegative.
    this->mooseWarning("The 'lulu_factor' parameter is larger than 1 / dim. This may cause "
                       "negative strain energy contributions for the HuHu-LuLu regularization.");
  }
}

template <bool is_ad>
GenericReal<is_ad>
SecondDerivativeRegularizationTempl<is_ad>::computeQpResidual()
{
  return _coefficient *
         regularization(_second_u[_qp], _second_test[_i][_qp], _regularization_type, _lulu_factor);
}

Real
SecondDerivativeRegularization::computeQpJacobian()
{
  return _coefficient *
         regularization(
             _second_phi[_j][_qp], _second_test[_i][_qp], _regularization_type, _lulu_factor);
}

template class SecondDerivativeRegularizationTempl<false>;
template class SecondDerivativeRegularizationTempl<true>;
