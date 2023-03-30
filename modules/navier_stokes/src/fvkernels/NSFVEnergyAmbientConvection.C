#include "NSFVEnergyAmbientConvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVEnergyAmbientConvection);

InputParameters
NSFVEnergyAmbientConvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Implements a solid-fluid ambient convection volumetric term "
      "proportional to the difference between the fluid and ambient temperatures : "
      "$q''' = \\alpha (T_{fluid} - T_{ambient})$.");
  params.addRequiredParam<MaterialPropertyName>(NS::alpha,
                                                "Name of the convective heat transfer coefficient");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "The ambient temperature");
  return params;
}

NSFVEnergyAmbientConvection::NSFVEnergyAmbientConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _alpha(getFunctor<ADReal>(NS::alpha)),
    _temp_ambient(getFunctor<ADReal>("T_ambient"))
{
}

ADReal
NSFVEnergyAmbientConvection::computeQpResidual()
{
  auto elem_arg = makeElemArg(_current_elem);
  const auto current_time = Moose::currentTimeFunctorArg();
  return _alpha(elem_arg, current_time) *
         (_var(elem_arg, current_time) - _temp_ambient(elem_arg, current_time));
}
