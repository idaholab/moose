#include "ADRadiativeHeatFluxRZBC.h"
#include "Function.h"

registerMooseObject("THMApp", ADRadiativeHeatFluxRZBC);

InputParameters
ADRadiativeHeatFluxRZBC::validParams()
{
  InputParameters params = ADRadiativeHeatFluxBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a cylindrical heat structure");

  return params;
}

ADRadiativeHeatFluxRZBC::ADRadiativeHeatFluxRZBC(const InputParameters & parameters)
  : ADRadiativeHeatFluxBC(parameters), RZSymmetry(parameters)
{
}

ADReal
ADRadiativeHeatFluxRZBC::computeQpResidual()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * ADRadiativeHeatFluxBC::computeQpResidual();
}
