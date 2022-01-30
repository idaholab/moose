#include "ADHSHeatFluxRZBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADHSHeatFluxRZBC);

InputParameters
ADHSHeatFluxRZBC::validParams()
{
  InputParameters params = ADHSHeatFluxBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Applies a specified heat flux to the side of a cylindrical heat structure");

  return params;
}

ADHSHeatFluxRZBC::ADHSHeatFluxRZBC(const InputParameters & parameters)
  : ADHSHeatFluxBC(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADHSHeatFluxRZBC::computeQpResidual()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * ADHSHeatFluxBC::computeQpResidual();
}
