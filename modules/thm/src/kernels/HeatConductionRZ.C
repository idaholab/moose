#include "HeatConductionRZ.h"

registerMooseObject("THMApp", HeatConductionRZ);

template <>
InputParameters
validParams<HeatConductionRZ>()
{
  InputParameters params = validParams<HeatConductionKernel>();
  params += validParams<RZSymmetry>();
  return params;
}

HeatConductionRZ::HeatConductionRZ(const InputParameters & parameters)
  : HeatConductionKernel(parameters), RZSymmetry(parameters)
{
}

Real
HeatConductionRZ::computeQpResidual()
{
  Real r = computeRadius(_q_point[_qp]);
  return 2. * libMesh::pi * r * HeatConductionKernel::computeQpResidual();
}

Real
HeatConductionRZ::computeQpJacobian()
{
  Real r = computeRadius(_q_point[_qp]);
  return 2. * libMesh::pi * r * HeatConductionKernel::computeQpJacobian();
}
