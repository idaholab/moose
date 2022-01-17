#include "ConvectionHeatTransferRZBC.h"

registerMooseObject("ThermalHydraulicsApp", ConvectionHeatTransferRZBC);

InputParameters
ConvectionHeatTransferRZBC::validParams()
{
  InputParameters params = ConvectionHeatTransferBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription("Convection BC for RZ domain in XY coordinate system");

  return params;
}

ConvectionHeatTransferRZBC::ConvectionHeatTransferRZBC(const InputParameters & parameters)
  : ConvectionHeatTransferBC(parameters), RZSymmetry(this, parameters)
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
