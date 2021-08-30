#include "ADVolumeJunctionAdvectionScalarKernel.h"
#include "ADVolumeJunctionBaseUserObject.h"

registerMooseObject("THMApp", ADVolumeJunctionAdvectionScalarKernel);

InputParameters
ADVolumeJunctionAdvectionScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();

  params.addRequiredParam<unsigned int>("equation_index", "Equation index");
  params.addRequiredParam<UserObjectName>("volume_junction_uo", "Volume junction user object name");

  params.addClassDescription(
      "Adds advective fluxes for the junction variables for a volume junction");

  return params;
}

ADVolumeJunctionAdvectionScalarKernel::ADVolumeJunctionAdvectionScalarKernel(
    const InputParameters & params)
  : ADScalarKernel(params),
    _equation_index(getParam<unsigned int>("equation_index")),
    _volume_junction_uo(getUserObject<ADVolumeJunctionBaseUserObject>("volume_junction_uo"))
{
  if (_var.order() > 1)
    mooseError(name(), ": This scalar kernel can be used only with first-order scalar variables.");
}

ADReal
ADVolumeJunctionAdvectionScalarKernel::computeQpResidual()
{
  return _volume_junction_uo.getResidual()[_equation_index];
}
