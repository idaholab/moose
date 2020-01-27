#include "FunctionSideIntegralRZ.h"

registerMooseObject("THMApp", FunctionSideIntegralRZ);

template <>
InputParameters
validParams<FunctionSideIntegralRZ>()
{
  InputParameters params = validParams<FunctionSideIntegral>();
  params += validParams<RZSymmetry>();

  params.addClassDescription(
      "Integrates a function over sides for RZ geometry modeled by XY domain");

  return params;
}

FunctionSideIntegralRZ::FunctionSideIntegralRZ(const InputParameters & parameters)
  : FunctionSideIntegral(parameters), RZSymmetry(parameters)
{
}

Real
FunctionSideIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * FunctionSideIntegral::computeQpIntegral();
}
