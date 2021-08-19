#include "ADConvectionHeatTransferRZBC.h"

registerMooseObject("THMApp", ADConvectionHeatTransferRZBC);

InputParameters
ADConvectionHeatTransferRZBC::validParams()
{
  InputParameters params = ADConvectionHeatTransferBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription("Convection BC for RZ domain in XY coordinate system");

  return params;
}

ADConvectionHeatTransferRZBC::ADConvectionHeatTransferRZBC(const InputParameters & parameters)
  : ADConvectionHeatTransferBC(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADConvectionHeatTransferRZBC::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADConvectionHeatTransferBC::computeQpResidual();
}
