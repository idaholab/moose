//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FERegularization.h"

registerMooseObject("SolidMechanicsApp", FERegularization);

namespace FERegularizationTools
{
MooseEnum
regularizationType()
{
  return MooseEnum("huhu lulu huhu_lulu");
}

FERegularizationType
regularizationType(const MooseEnum & regularization)
{
  if (regularization == "huhu")
    return FERegularizationType::Huhu;
  if (regularization == "lulu")
    return FERegularizationType::Lulu;
  if (regularization == "huhu_lulu")
    return FERegularizationType::HuhuLulu;

  mooseError("Unknown FE regularization type '", regularization, "'.");
}

Real
defaultLuluFactor(const unsigned int dim)
{
  return 1.0 / static_cast<Real>(dim);
}
}

InputParameters
FERegularization::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Adds a finite-element regularization term using HuHu, LuLu, or "
                             "HuHu-LuLu Hessian/Laplacian contractions.");
  params.addRequiredParam<MooseEnum>(
      "regularization",
      FERegularizationTools::regularizationType(),
      "Regularization type to apply: 'huhu' for Hessian-Hessian, 'lulu' for "
      "Laplacian-Laplacian, or 'huhu_lulu' for HuHu minus a scaled LuLu correction.");
  params.addRequiredRangeCheckedParam<Real>(
      "coefficient", "coefficient >= 0", "Coefficient multiplying the regularization term.");
  params.addParam<Real>("lulu_factor",
                        "Factor multiplying the LuLu correction for 'huhu_lulu'. If omitted, "
                        "defaults to 1 / dim, where dim is the mesh dimension.");
  return params;
}

FERegularization::FERegularization(const InputParameters & parameters)
  : Kernel(parameters),
    _regularization_type(
        FERegularizationTools::regularizationType(getParam<MooseEnum>("regularization"))),
    _coefficient(getParam<Real>("coefficient")),
    _dim(_mesh.dimension()),
    _lulu_factor(isParamValid("lulu_factor") ? getParam<Real>("lulu_factor")
                                             : FERegularizationTools::defaultLuluFactor(_dim)),
    _second_u(_var.secondSln()),
    _second_phi(_var.secondPhi()),
    _second_test(secondTest())
{
  if (_regularization_type == FERegularizationType::HuhuLulu && _dim == 1 &&
      !isParamValid("lulu_factor"))
  {
    // In 1D the Hessian and Laplacian contractions are the same, so the default factor makes the
    // combined HuHu-LuLu contribution vanish identically.
    paramError("regularization",
               "The 'huhu_lulu' regularization is not supported in 1D. HuHu and LuLu are "
               "identical in 1D when the default LuLu factor is used, so use 'huhu' or 'lulu' "
               "directly for one-dimensional problems.");
  }

  if (_regularization_type == FERegularizationType::HuhuLulu &&
      _lulu_factor > FERegularizationTools::defaultLuluFactor(_dim))
  {
    // The bound c_L <= 1 / dim keeps the HuHu-LuLu quadratic form nonnegative.
    mooseWarning("The 'lulu_factor' parameter is larger than 1 / dim. This may cause negative "
                 "strain energy contributions for the HuHu-LuLu regularization.");
  }
}

Real
FERegularization::computeQpResidual()
{
  return _coefficient *
         FERegularizationTools::regularization(
             _second_u[_qp], _second_test[_i][_qp], _dim, _regularization_type, _lulu_factor);
}

Real
FERegularization::computeQpJacobian()
{
  return _coefficient *
         FERegularizationTools::regularization(
             _second_phi[_j][_qp], _second_test[_i][_qp], _dim, _regularization_type, _lulu_factor);
}
