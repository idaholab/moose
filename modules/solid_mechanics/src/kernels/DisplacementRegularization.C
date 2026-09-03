//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplacementRegularization.h"

registerMooseObject("SolidMechanicsApp", DisplacementRegularization);
registerMooseObject("SolidMechanicsApp", ADDisplacementRegularization);

namespace DisplacementRegularizationTools
{
MooseEnum
regularizationType()
{
  return MooseEnum("huhu lulu huhu_lulu");
}

DisplacementRegularizationType
regularizationType(const MooseEnum & regularization)
{
  if (regularization == "huhu")
    return DisplacementRegularizationType::Huhu;
  if (regularization == "lulu")
    return DisplacementRegularizationType::Lulu;
  if (regularization == "huhu_lulu")
    return DisplacementRegularizationType::HuhuLulu;

  mooseError("Unknown displacement regularization type '", regularization, "'.");
}

Real
defaultLuluFactor(const unsigned int dim)
{
  return 1.0 / static_cast<Real>(dim);
}
}

template <bool is_ad>
InputParameters
DisplacementRegularizationTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  if constexpr (is_ad)
    params.addClassDescription("Adds a solid mechanics displacement regularization term using "
                               "HuHu, LuLu, or HuHu-LuLu Hessian/Laplacian contractions. The "
                               "Jacobian is computed using automatic differentiation.");
  else
    params.addClassDescription("Adds a solid mechanics displacement regularization term using "
                               "HuHu, LuLu, or HuHu-LuLu Hessian/Laplacian contractions.");
  params.addRequiredParam<MooseEnum>(
      "regularization",
      DisplacementRegularizationTools::regularizationType(),
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
DisplacementRegularizationTempl<is_ad>::DisplacementRegularizationTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _regularization_type(DisplacementRegularizationTools::regularizationType(
        this->template getParam<MooseEnum>("regularization"))),
    _coefficient(this->template getParam<Real>("coefficient")),
    _dim(this->_mesh.dimension()),
    _lulu_factor(this->isParamValid("lulu_factor")
                     ? this->template getParam<Real>("lulu_factor")
                     : DisplacementRegularizationTools::defaultLuluFactor(_dim)),
    _second_u(DisplacementRegularizationTools::secondSln<is_ad>(this->_var)),
    _second_test(this->secondTest())
{
  if (_regularization_type == DisplacementRegularizationType::HuhuLulu && _dim == 1 &&
      !this->isParamValid("lulu_factor"))
  {
    // In 1D the Hessian and Laplacian contractions are the same, so the default factor makes the
    // combined HuHu-LuLu contribution vanish identically.
    this->paramError("regularization",
                     "The 'huhu_lulu' regularization is not supported in 1D. HuHu and LuLu are "
                     "identical in 1D when the default LuLu factor is used, so use 'huhu' or "
                     "'lulu' directly for one-dimensional problems.");
  }

  if (_regularization_type == DisplacementRegularizationType::HuhuLulu &&
      _lulu_factor > DisplacementRegularizationTools::defaultLuluFactor(_dim))
  {
    // The bound c_L <= 1 / dim keeps the HuHu-LuLu quadratic form nonnegative.
    this->mooseWarning("The 'lulu_factor' parameter is larger than 1 / dim. This may cause "
                       "negative strain energy contributions for the HuHu-LuLu regularization.");
  }
}

template <bool is_ad>
GenericReal<is_ad>
DisplacementRegularizationTempl<is_ad>::computeQpResidual()
{
  return _coefficient *
         DisplacementRegularizationTools::regularization(
             _second_u[_qp], _second_test[_i][_qp], _dim, _regularization_type, _lulu_factor);
}

InputParameters
DisplacementRegularization::validParams()
{
  return DisplacementRegularizationTempl<false>::validParams();
}

DisplacementRegularization::DisplacementRegularization(const InputParameters & parameters)
  : DisplacementRegularizationTempl<false>(parameters), _second_phi(_var.secondPhi())
{
}

Real
DisplacementRegularization::computeQpJacobian()
{
  return _coefficient *
         DisplacementRegularizationTools::regularization(
             _second_phi[_j][_qp], _second_test[_i][_qp], _dim, _regularization_type, _lulu_factor);
}

template class DisplacementRegularizationTempl<false>;
template class DisplacementRegularizationTempl<true>;
