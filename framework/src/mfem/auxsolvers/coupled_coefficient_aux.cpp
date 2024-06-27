#include "coupled_coefficient_aux.hpp"

namespace hephaestus
{

CoupledCoefficient::CoupledCoefficient(const hephaestus::InputParameters & params)
  : _coupled_var_name(params.GetParam<std::string>("CoupledVariableName"))
{
}

void
CoupledCoefficient::Init(const hephaestus::GridFunctions & gridfunctions,
                         hephaestus::Coefficients & coefficients)
{
  _gf = gridfunctions.Get(_coupled_var_name);
}

double
CoupledCoefficient::Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
{
  return _gf->GetValue(T, ip);
}

} // namespace hephaestus
