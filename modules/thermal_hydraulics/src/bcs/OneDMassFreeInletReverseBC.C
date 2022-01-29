#include "OneDMassFreeInletReverseBC.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", OneDMassFreeInletReverseBC);

InputParameters
OneDMassFreeInletReverseBC::validParams()
{
  InputParameters params = OneDMassFreeBC::validParams();
  params.addRequiredParam<bool>("reversible",
                                "true if the boundary condition is reversible, otherwise false.");
  return params;
}

OneDMassFreeInletReverseBC::OneDMassFreeInletReverseBC(const InputParameters & parameters)
  : OneDMassFreeBC(parameters),
    _reversible(getParam<bool>("reversible")),
    _arhouA_old(coupledValueOld("arhouA"))
{
}

bool
OneDMassFreeInletReverseBC::shouldApply()
{
  return !_reversible || THM::isOutlet(_arhouA_old[0], _normal);
}
