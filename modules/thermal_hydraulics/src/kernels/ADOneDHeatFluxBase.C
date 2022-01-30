#include "ADOneDHeatFluxBase.h"
#include "ADHeatFluxFromHeatStructureBaseUserObject.h"

InputParameters
ADOneDHeatFluxBase::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "q_uo", "The name of the user object that computed the heat flux");
  return params;
}

ADOneDHeatFluxBase::ADOneDHeatFluxBase(const InputParameters & parameters)
  : ADKernel(parameters), _q_uo(getUserObject<ADHeatFluxFromHeatStructureBaseUserObject>("q_uo"))
{
}
