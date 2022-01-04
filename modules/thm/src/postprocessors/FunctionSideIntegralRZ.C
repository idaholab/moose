#include "FunctionSideIntegralRZ.h"

registerMooseObject("THMApp", FunctionSideIntegralRZ);

InputParameters
FunctionSideIntegralRZ::validParams()
{
  InputParameters params = FunctionSideIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a function over sides for RZ geometry modeled by XY domain");

  return params;
}

FunctionSideIntegralRZ::FunctionSideIntegralRZ(const InputParameters & parameters)
  : FunctionSideIntegral(parameters), RZSymmetry(this, parameters)
{
}

Real
FunctionSideIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * FunctionSideIntegral::computeQpIntegral();
}
