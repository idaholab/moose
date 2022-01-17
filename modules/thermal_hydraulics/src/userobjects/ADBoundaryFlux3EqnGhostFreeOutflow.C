#include "ADBoundaryFlux3EqnGhostFreeOutflow.h"
#include "THMIndices3Eqn.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostFreeOutflow);

InputParameters
ADBoundaryFlux3EqnGhostFreeOutflow::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Outflow boundary flux from a ghost cell for the 1-D, 1-phase, "
                             "variable-area Euler equations");

  return params;
}

ADBoundaryFlux3EqnGhostFreeOutflow::ADBoundaryFlux3EqnGhostFreeOutflow(
    const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters)
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostFreeOutflow::getGhostCellSolution(const std::vector<ADReal> & U1) const
{
  return U1;
}
