#include "GeneralizedCircumference.h"

registerMooseObject("THMApp", GeneralizedCircumference);

template <>
InputParameters
validParams<GeneralizedCircumference>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<FunctionName>("area_function", "function to compute the cross section");
  return params;
}

GeneralizedCircumference::GeneralizedCircumference(const InputParameters & parameters)
  : Function(parameters), FunctionInterface(this), _area_func(getFunction("area_function"))
{
}

Real
GeneralizedCircumference::value(Real t, const Point & p)
{
  RealVectorValue gradA = _area_func.gradient(t, p);
  Real dAdx2 = gradA(0) * gradA(0);

  // Here, we assume a pipe with circular cross section.
  return std::sqrt(4. * libMesh::pi * _area_func.value(t, p) + dAdx2);
}

RealVectorValue
GeneralizedCircumference::gradient(Real /*t*/, const Point & /*p*/)
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
