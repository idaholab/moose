#include "FunctionElementIntegralRZ.h"

registerMooseObject("THMApp", FunctionElementIntegralRZ);

template <>
InputParameters
validParams<FunctionElementIntegralRZ>()
{
  InputParameters params = validParams<FunctionElementIntegral>();
  params += validParams<RZSymmetry>();

  params.addClassDescription(
      "Integrates a function over elements for RZ geometry modeled by XY domain");

  return params;
}

FunctionElementIntegralRZ::FunctionElementIntegralRZ(const InputParameters & parameters)
  : FunctionElementIntegral(parameters), RZSymmetry(parameters)
{
}

Real
FunctionElementIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * FunctionElementIntegral::computeQpIntegral();
}
