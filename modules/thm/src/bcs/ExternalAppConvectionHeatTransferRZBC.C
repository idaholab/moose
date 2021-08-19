#include "ExternalAppConvectionHeatTransferRZBC.h"

registerMooseObject("THMApp", ExternalAppConvectionHeatTransferRZBC);

InputParameters
ExternalAppConvectionHeatTransferRZBC::validParams()
{
  InputParameters params = ExternalAppConvectionHeatTransferBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Convection BC from an external application for RZ domain in XY coordinate system");

  return params;
}

ExternalAppConvectionHeatTransferRZBC::ExternalAppConvectionHeatTransferRZBC(
    const InputParameters & parameters)
  : ExternalAppConvectionHeatTransferBC(parameters), RZSymmetry(this, parameters)
{
}

Real
ExternalAppConvectionHeatTransferRZBC::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ExternalAppConvectionHeatTransferBC::computeQpResidual();
}

Real
ExternalAppConvectionHeatTransferRZBC::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ExternalAppConvectionHeatTransferBC::computeQpJacobian();
}
