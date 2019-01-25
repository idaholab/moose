#include "HeatConductionTimeDerivativeRZ.h"

registerMooseObject("THMApp", HeatConductionTimeDerivativeRZ);

template <>
InputParameters
validParams<HeatConductionTimeDerivativeRZ>()
{
  InputParameters params = validParams<HeatConductionTimeDerivative>();
  params += validParams<RZSymmetry>();
  return params;
}

HeatConductionTimeDerivativeRZ::HeatConductionTimeDerivativeRZ(const InputParameters & parameters)
  : HeatConductionTimeDerivative(parameters), RZSymmetry(parameters)
{
}

Real
HeatConductionTimeDerivativeRZ::computeQpResidual()
{
  Real r = computeRadius(_q_point[_qp]);
  return 2. * libMesh::pi * r * HeatConductionTimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivativeRZ::computeQpJacobian()
{
  Real r = computeRadius(_q_point[_qp]);
  return 2. * libMesh::pi * r * HeatConductionTimeDerivative::computeQpJacobian();
}
