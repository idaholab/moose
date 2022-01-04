#include "FunctionElementIntegralRZ.h"

registerMooseObject("THMApp", FunctionElementIntegralRZ);

InputParameters
FunctionElementIntegralRZ::validParams()
{
  InputParameters params = FunctionElementIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a function over elements for RZ geometry modeled by XY domain");

  return params;
}

FunctionElementIntegralRZ::FunctionElementIntegralRZ(const InputParameters & parameters)
  : FunctionElementIntegral(parameters), RZSymmetry(this, parameters)
{
}

Real
FunctionElementIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * FunctionElementIntegral::computeQpIntegral();
}
