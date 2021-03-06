#include "CosTheta.h"

registerMooseObject("ElkApp", CosTheta);

InputParameters
CosTheta::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Function for cosine(theta) (where theta is in degrees) for use in reflection problems.");
  params.addRequiredParam<Real>("theta", "Angle (in degrees).");
  return params;
}

CosTheta::CosTheta(const InputParameters & parameters)
  : Function(parameters),

    _theta(getParam<Real>("theta"))
{
}

Real
CosTheta::value(Real /*t*/, const Point & /*p*/) const
{
  return std::cos(_theta * libMesh::pi / 180.);
}
