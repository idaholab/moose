#include "RadiativeHeatFluxRZBC.h"
#include "Function.h"

registerMooseObject("THMApp", RadiativeHeatFluxRZBC);

template <>
InputParameters
validParams<RadiativeHeatFluxRZBC>()
{
  InputParameters params = validParams<RadiativeHeatFluxBC>();
  params += validParams<RZSymmetry>();

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a cylindrical heat structure");

  return params;
}

RadiativeHeatFluxRZBC::RadiativeHeatFluxRZBC(const InputParameters & parameters)
  : RadiativeHeatFluxBC(parameters), RZSymmetry(parameters)
{
}

Real
RadiativeHeatFluxRZBC::computeQpResidual()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * RadiativeHeatFluxBC::computeQpResidual();
}

Real
RadiativeHeatFluxRZBC::computeQpJacobian()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * RadiativeHeatFluxBC::computeQpJacobian();
}
