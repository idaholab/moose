#include "OneDFieldCoeff.h"

registerMooseObject("ElkApp", OneDFieldCoeff);

InputParameters
OneDFieldCoeff::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Coefficient for field term in Helmholtz wave equation, for 1D Ez-polarized plane wave.");
  params.addRequiredParam<Real>("theta", "Angle of wave incidence, in degrees");
  params.addParam<FunctionName>("epsR", 1.0, "relative permittivity");
  params.addParam<FunctionName>("inverseMuR", 1.0, "1 over relative permeability");
  return params;
}

OneDFieldCoeff::OneDFieldCoeff(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _theta(getParam<Real>("theta")),

    _eps_r(getFunction("epsR")),
    _inverse_mu_r(getFunction("inverseMuR"))

{
}

Real
OneDFieldCoeff::value(Real t, const Point & p) const
{
  return _eps_r.value(t, p) -
         _inverse_mu_r.value(t, p) * std::pow(std::sin(_theta * 2 * libMesh::pi / 360.), 2);
}
