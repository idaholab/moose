#include "ADExternalAppConvectionHeatTransferRZBC.h"

registerMooseObject("ThermalHydraulicsApp", ADExternalAppConvectionHeatTransferRZBC);

InputParameters
ADExternalAppConvectionHeatTransferRZBC::validParams()
{
  InputParameters params = ADExternalAppConvectionHeatTransferBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Convection BC from an external application for RZ domain in XY coordinate system");

  return params;
}

ADExternalAppConvectionHeatTransferRZBC::ADExternalAppConvectionHeatTransferRZBC(
    const InputParameters & parameters)
  : ADExternalAppConvectionHeatTransferBC(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADExternalAppConvectionHeatTransferRZBC::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADExternalAppConvectionHeatTransferBC::computeQpResidual();
}
