#include "HSHeatFluxRZBC.h"
#include "Function.h"

registerMooseObject("THMApp", HSHeatFluxRZBC);

template <>
InputParameters
validParams<HSHeatFluxRZBC>()
{
  InputParameters params = validParams<HSHeatFluxBC>();
  params += validParams<RZSymmetry>();

  params.addClassDescription(
      "Applies a specified heat flux to the side of a cylindrical heat structure");

  return params;
}

HSHeatFluxRZBC::HSHeatFluxRZBC(const InputParameters & parameters)
  : HSHeatFluxBC(parameters), RZSymmetry(parameters)
{
}

Real
HSHeatFluxRZBC::computeQpResidual()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * HSHeatFluxBC::computeQpResidual();
}
