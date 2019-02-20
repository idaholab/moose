#include "CircularAreaHydraulicDiameterFunction.h"

registerMooseObject("THMApp", CircularAreaHydraulicDiameterFunction);

template <>
InputParameters
validParams<CircularAreaHydraulicDiameterFunction>()
{
  InputParameters params = validParams<Function>();

  params.addRequiredParam<FunctionName>("area_function", "Area function");

  params.addClassDescription(
      "Computes hydraulic diameter for a circular area from its area function");

  return params;
}

CircularAreaHydraulicDiameterFunction::CircularAreaHydraulicDiameterFunction(
    const InputParameters & parameters)
  : Function(parameters), FunctionInterface(this), _area_function(getFunction("area_function"))
{
}

Real
CircularAreaHydraulicDiameterFunction::value(Real t, const Point & p)
{
  const Real A = _area_function.value(t, p);
  return std::sqrt(4.0 * A / M_PI);
}

RealVectorValue
CircularAreaHydraulicDiameterFunction::gradient(Real /*t*/, const Point & /*p*/)
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
