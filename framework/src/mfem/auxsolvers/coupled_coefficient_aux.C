#include "coupled_coefficient_aux.h"

namespace platypus
{

CoupledCoefficient::CoupledCoefficient(const platypus::InputParameters & params)
  : _coupled_var_name(params.GetParam<std::string>("CoupledVariableName"))
{
}

void
CoupledCoefficient::Init(const platypus::GridFunctions & gridfunctions,
                         platypus::Coefficients & coefficients)
{
  _gf = gridfunctions.Get(_coupled_var_name);
}

double
CoupledCoefficient::Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
{
  return _gf->GetValue(T, ip);
}

} // namespace platypus
