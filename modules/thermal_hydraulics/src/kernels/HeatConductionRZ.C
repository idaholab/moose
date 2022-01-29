#include "HeatConductionRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatConductionRZ);

InputParameters
HeatConductionRZ::validParams()
{
  InputParameters params = HeatConductionKernel::validParams();
  params += RZSymmetry::validParams();
  return params;
}

HeatConductionRZ::HeatConductionRZ(const InputParameters & parameters)
  : HeatConductionKernel(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatConductionRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionKernel::computeQpResidual();
}

Real
HeatConductionRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionKernel::computeQpJacobian();
}
