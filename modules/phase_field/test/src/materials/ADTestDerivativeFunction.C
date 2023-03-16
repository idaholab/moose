//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTestDerivativeFunction.h"

registerMooseObject("PhaseFieldTestApp", ADTestDerivativeFunction);

InputParameters
ADTestDerivativeFunction::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material that implements the a function of one variable and its first derivative.");
  MooseEnum functionEnum("F1 F2 F3");
  params.addRequiredParam<MooseEnum>("function",
                                     functionEnum,
                                     "F1 = 2 op[0]^2 (1 - op[0])^2 - 0.2 op[0]; "
                                     "F2 = 0.1 op[0]^2 + op[1]^2; "
                                     "F3 = op[0] * op[1]");
  params.addParam<MaterialPropertyName>("f_name", "F", "function property name");
  params.addRequiredCoupledVar("op", "Order parameter variables");
  return params;
}

ADTestDerivativeFunction::ADTestDerivativeFunction(const InputParameters & parameters)
  : Material(parameters),
    _function(getParam<MooseEnum>("function").template getEnum<FunctionEnum>()),
    _op(adCoupledValues("op")),
    _f_name(getParam<MaterialPropertyName>("f_name")),
    _prop_F(declareADProperty<Real>(_f_name)),
    _prop_dFdop(coupledComponents("op"))
{
  for (std::size_t i = 0; i < _op.size(); ++i)
    _prop_dFdop[i] =
        &declareADProperty<Real>(derivativePropertyNameFirst(_f_name, this->coupledName("op", i)));

  if (_function == FunctionEnum::F1 && _op.size() != 1)
    paramError("op", "Specify exactly one variable to an F1 type function.");
  if (_function == FunctionEnum::F2 && _op.size() != 2)
    paramError("op", "Specify exactly two variables to an F2 type function.");
  if (_function == FunctionEnum::F3 && _op.size() != 2)
    paramError("op", "Specify exactly two variables to an F3 type function.");
}

void
ADTestDerivativeFunction::computeQpProperties()
{
  const ADReal & a = (*_op[0])[_qp];

  switch (_function)
  {
    case FunctionEnum::F1:
      _prop_F[_qp] = 2.0 * a * a * (1.0 - a) * (1.0 - a) - 0.2 * a;
      (*_prop_dFdop[0])[_qp] = 4.0 * a * a * (a - 1.0) + 4.0 * a * (1.0 - a) * (1.0 - a) - 0.2;
      break;

    case FunctionEnum::F2:
    {
      const ADReal & b = (*_op[1])[_qp];
      _prop_F[_qp] = 0.1 * a * a + b * b;
      (*_prop_dFdop[0])[_qp] = 0.2 * a;
      (*_prop_dFdop[1])[_qp] = 2.0 * b;
      break;
    }

    case FunctionEnum::F3:
    {
      const ADReal & b = (*_op[1])[_qp];
      _prop_F[_qp] = a * b;
      (*_prop_dFdop[0])[_qp] = b;
      (*_prop_dFdop[1])[_qp] = a;
      break;
    }

    default:
      mooseError("Invalid function enum value");
  }
}
