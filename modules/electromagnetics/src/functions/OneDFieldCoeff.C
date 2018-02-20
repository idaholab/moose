#include "OneDFieldCoeff.h"

template<>
InputParameters validParams<OneDFieldCoeff>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Coefficient for field term in Helmholtz wave equation, for 1D Ez-polarized plane wave.");
  params.addRequiredParam<Real>("theta","Angle of wave incidence, in degrees");
  params.addParam<FunctionName>("epsR", 1.0, "relative permittivity");
  params.addParam<FunctionName>("inverseMuR", 1.0, "1 over relative permeability");
  return params;
}

OneDFieldCoeff::OneDFieldCoeff(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _theta(getParam<Real>("theta")),

    _epsR(getFunction("epsR")),
    _InverseMuR(getFunction("inverseMuR"))

{}

Real
OneDFieldCoeff::value(Real t, const Point & p)
{
  return _epsR.value(t,p) - _InverseMuR.value(t,p) * std::pow(std::sin(_theta * 2 * libMesh::pi / 360.),2);
}
