#include "ConvectionHeatTransferRZBC.h"

registerMooseObject("THMApp", ConvectionHeatTransferRZBC);

template <>
InputParameters
validParams<ConvectionHeatTransferRZBC>()
{
  InputParameters params = validParams<ConvectionHeatTransferBC>();
  params += validParams<RZSymmetry>();

  params.addClassDescription("Convection BC for RZ domain in XY coordinate system");

  return params;
}

ConvectionHeatTransferRZBC::ConvectionHeatTransferRZBC(const InputParameters & parameters)
  : ConvectionHeatTransferBC(parameters), RZSymmetry(parameters)
{
}

Real
ConvectionHeatTransferRZBC::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ConvectionHeatTransferBC::computeQpResidual();
}

Real
ConvectionHeatTransferRZBC::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ConvectionHeatTransferBC::computeQpJacobian();
}
